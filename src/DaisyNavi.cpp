/*
 * Copyright (C) 2012 Kolibre
 *
 * This file is part of kolibre-clientcore.
 *
 * Kolibre-clientcore is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Kolibre-clientcore is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with kolibre-clientcore. If not, see <http://www.gnu.org/licenses/>.
 */

#include "DaisyNavi.h"
#include "Navi.h"
#include "Menu/ContextMenuNode.h"
#include "Menu/BookInfoNode.h"
#include "Menu/NarratedNode.h"
#include "Menu/GotoPageNode.h"
#include "Menu/GotoTimeNode.h"
#include "Menu/GotoPercentNode.h"
#include "Commands/InternalCommands.h"
#include "Commands/JumpCommand.h"
#include "CommandQueue2/CommandQueue.h"
#include "ClientCore.h"
#include "Defines.h"
#include "config.h"

#include <AmisError.h>
#include <Nodes/MenuNode.h>

#include <unistd.h>
#include <libintl.h>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr daisyNaviLog(log4cxx::Logger::getLogger("kolibre.clientcore.daisynavi"));

using namespace amis;
using namespace naviengine;

static long lastsecond = -1;
static int lastpage = -1;
static int lastsection = -1;

bool DaisyNavi::select(NaviEngine& navi)
{
    return process(navi, COMMAND_DOWN);
}

bool DaisyNavi::selectByUri(NaviEngine& navi, std::string uri)
{
    // Store current position in case select fails
    Player::Instance()->pause();

    if (bPlaybackIsPaused)
    {
        bPlaybackIsPaused = false;
        Narrator::Instance()->setPushCommandFinished(true);
    }

    // GotoNode in contextmenu will do the talking if jump comes from there
    // (not anymore)
    if (not bContextMenuIsOpen)
        Narrator::Instance()->play(_N("jumping to"));

    // Perform the jump
    if (not dh->goToId(uri))
    {
        //TODO: set amis error to GOTO_FAIL
        //TODO: make error handle routine in process a method and let it handle the error
        Narrator::Instance()->play(_N("jump to position failed"));
        LOG4CXX_ERROR(daisyNaviLog, "Jump to '" << uri << "' failed");
        Player::Instance()->reopen();
        return false;
    }

    return true;
}

bool DaisyNavi::up(NaviEngine& navi)
{
    if (not process(navi, COMMAND_UP))
    {
        Narrator::Instance()->setPushCommandFinished(false); // Let the parent enable callbacks if he needs them.
        bContextMenuIsOpen = false;
        // disconnect from player slots
        if (playerMsgCon.connected())
        {
            playerMsgCon.disconnect();
        }
        if (playerTimeCon.connected())
        {
            playerTimeCon.disconnect();
        }
        return false;
    }

    return true;
}

bool DaisyNavi::next(NaviEngine& navi)
{
    return process(navi, COMMAND_RIGHT);
}

bool DaisyNavi::prev(NaviEngine& navi)
{
    return process(navi, COMMAND_LEFT);
}

bool DaisyNavi::menu(NaviEngine& navi)
{
    MenuNode* contextMenu = navi.buildContextMenu();

    DaisyHandler::BookInfo *bookInfo = dh->getBookInfo();
    player->pause();

    //GOTO
    if (bookInfo->mTotalTime.tm_hour != -1 && bookInfo->mTotalTime.tm_min != -1 && bookInfo->mTotalTime.tm_sec != -1)
    {
        ContextMenuNode* jumpNode = new ContextMenuNode(_N("jump to"), _N("opening jump to"));
        contextMenu->addNode(jumpNode);
        {
            const unsigned int totalTime = bookInfo->mTotalTime.tm_hour * 60 * 60 + bookInfo->mTotalTime.tm_min * 60 + bookInfo->mTotalTime.tm_sec;
            AnyNode* percentJump = new GotoPercentNode(totalTime, this);
            jumpNode->addNode(percentJump);

            AnyNode* timeJump = new GotoTimeNode(totalTime, this);
            jumpNode->addNode(timeJump);

            if (bookInfo->hasPages && bookInfo->mNormalPages > 0)
            {
                // Sum the total of pages.
                int pageCount = bookInfo->mNormalPages + bookInfo->mSpecialPages + bookInfo->mFrontPages;
                AnyNode* pageJump = new GotoPageNode(pageCount, this);
                jumpNode->addNode(pageJump);
            }

        }
    }

    BookInfoNode* infoNode = new BookInfoNode(_N("content info"), _N("loading content info"));
    buildInfoNode(infoNode);
    contextMenu->addNode(infoNode);

    bool success = navi.openMenu(contextMenu, true);
    if (success)
    {
        bContextMenuIsOpen = true;
        Narrator::Instance()->setPushCommandFinished(false); // While menu is open don't send NARRATORFINISHED COMMANDS TO THE QUEUE
    }

    return success;
}

void DaisyNavi::buildInfoNode(BookInfoNode* info)
{

    DaisyHandler::BookInfo *bookInfo = dh->getBookInfo();
    DaisyHandler::PosInfo *posInfo = dh->getPosInfo();
    int childNumber = 0;

    info->name_ = _N("content info");
    info->info_ = _("navigate using left and right arrows. if no input is given, all information will be read automatically");

    // TITLE
    NarratedNode* title = new NarratedNode(_N("content title"), ++childNumber);
    title->appendNarratedString(parent_->name_); // This code belongs in DaisyOnlineBookNode.
    title->appendNarratedString(_N("longpause"));
    info->addNode(title);
    // TIMEINFO
    if (bookInfo->hasTime)
    {
        NarratedNode* a = new NarratedNode(_N("time info"), ++childNumber);
        info->addNode(a);
        a->appendNarratedString(_N("shortpause"));

        if (posInfo->hasCurrentTime)
        {
            if (getCurrentPercent() == 1)
            {
                a->appendNarratedString(_N("{1} percent read"), "1", getCurrentPercent());
            }
            else
            {
                a->appendNarratedString(_N("{2} percent read"), "2", getCurrentPercent());
            }
            a->appendNarratedString(_N("current position"));
            int currentTime = getCurrentTime();
            a->appendNarratedDuration(currentTime / 3600, (currentTime / 60) % 60, currentTime % 60);
            a->appendNarratedString(_N("shortpause"));
        }

        a->appendNarratedString(_N("total time"));
        a->appendNarratedDuration(bookInfo->mTotalTime.tm_hour, bookInfo->mTotalTime.tm_min, bookInfo->mTotalTime.tm_sec);

        a->appendNarratedString(_N("longpause"));
    }

    // PAGEINFO
    if (bookInfo->hasPages)
    {
        NarratedNode* a = new NarratedNode(_N("page info"), ++childNumber);
        info->addNode(a);
        a->appendNarratedString(_N("shortpause"));
        const DaisyHandler::PosInfo* posInfo = dh->getPosInfo();
        string label = getPageLabel(posInfo->currentPageIdx);
        if (atoi(label.c_str()) > 0)
        {
            a->appendNarratedString(_N("current page"));
            a->appendNarratedString(atoi(label.c_str()));
            a->appendNarratedString(_N("shortpause"));
        }
        a->appendNarratedString(_N("highest page number in this book"));
        a->appendNarratedString(bookInfo->mMaxPageNum);
        a->appendNarratedString(_N("longpause"));
    }

    // NAVIINFO
    {
        NarratedNode* a = new NarratedNode(_N("navigation info"), ++childNumber);
        info->addNode(a);
        a->appendNarratedString(_N("shortpause"));
        a->appendNarratedString(_N("publication has"));
        if (bookInfo->mLevels == 1)
        {
            a->appendNarratedString(_N("{1} navigation level"), "1", bookInfo->mLevels);
            a->appendNarratedString(_N("and"));
        }
        else
        {
            a->appendNarratedString(_N("{2} navigation levels"), "2", bookInfo->mLevels);
            a->appendNarratedString(_N("and"));
        }

        a->appendNarratedString(_N("shortpause"));

        if (bookInfo->mTocItems == 1)
        {
            a->appendNarratedString(_N("{1} navigation point"), "1", bookInfo->mTocItems);
        }
        else
        {
            a->appendNarratedString(_N("{2} navigation points"), "2", bookInfo->mTocItems);
        }
        a->appendNarratedString(_N("longpause"));
    }

    // DATES
    if (bookInfo->hasSourceDate)
    {
        NarratedNode* a = new NarratedNode(_N("source material was created"), ++childNumber);
        info->addNode(a);
        a->appendNarratedString(_N("shortpause"));

        if (bookInfo->mSourceDate.tm_mon == -1 || bookInfo->mSourceDate.tm_mday == -1)
        {
            a->appendNarratedString(_N("year"));
            a->appendNarratedYear(bookInfo->mSourceDate.tm_year + 1900);
        }
        else
        {
            a->appendNarratedDate(bookInfo->mSourceDate.tm_mday, bookInfo->mSourceDate.tm_mon + 1, bookInfo->mSourceDate.tm_year + 1900);
        }
        a->appendNarratedString(_N("shortpause"));
    }

    if (bookInfo->hasProdDate)
    {
        NarratedNode* a = new NarratedNode(_N("content was recorded"), ++childNumber);
        info->addNode(a);
        a->appendNarratedString(_N("shortpause"));

        if (bookInfo->mProdDate.tm_mon == -1 || bookInfo->mProdDate.tm_mday == -1)
        {
            a->appendNarratedString(_N("year"));
            a->appendNarratedYear(bookInfo->mProdDate.tm_year + 1900);
        }
        else
        {
            a->appendNarratedDate(bookInfo->mProdDate.tm_mday, bookInfo->mProdDate.tm_mon + 1, bookInfo->mProdDate.tm_year + 1900);
        }
        a->appendNarratedString(_N("shortpause"));
    }

    if (bookInfo->hasRevisionDate)
    {
        NarratedNode* a = new NarratedNode(_N("content has revision number"), ++childNumber);
        info->addNode(a);
        a->appendNarratedString(_N("shortpause"));
        if (bookInfo->mRevisionNumber != -1)
        {
            a->appendNarratedString(1);
        }
        else
        {
            a->appendNarratedString(bookInfo->mRevisionNumber);
        }
        a->appendNarratedString(_N("and was last changed on"));
        if (bookInfo->mRevisionDate.tm_mon == -1 || bookInfo->mRevisionDate.tm_mday == -1)
        {
            a->appendNarratedString(_N("year"));
            a->appendNarratedYear(bookInfo->mRevisionDate.tm_year + 1900);
        }
        else
        {
            a->appendNarratedDate(bookInfo->mRevisionDate.tm_mday, bookInfo->mRevisionDate.tm_mon + 1, bookInfo->mRevisionDate.tm_year + 1900);
        }
        a->appendNarratedString(_N("longpause"));
    }

    // FORMAT
    if (bookInfo->hasDaisyType)
    {
        NarratedNode* a = new NarratedNode(_N("content type"), ++childNumber);
        info->addNode(a);
        a->appendNarratedString(_N("shortpause"));
        switch (bookInfo->mDaisyType)
        {
        case 200:
            a->appendNarratedString(_N("daisy2.00"));
            break;
        case 202:
            a->appendNarratedString(_N("daisy2.02"));
            break;
        case 2002:
            a->appendNarratedString(_N("daisyNISO2002"));
            break;
        case 2005:
            a->appendNarratedString(_N("daisyNISO2005"));
            break;
        }

        a->appendNarratedString(_N("shortpause"));

        if (bookInfo->mContentType != -1)
        {
            NarratedNode* a = new NarratedNode(_N("daisy type"), ++childNumber);
            info->addNode(a);
            a->appendNarratedString(_N("shortpause"));
            switch (bookInfo->mContentType)
            {
            case 1:
                a->appendNarratedString(_N("daisytype1"));
                break;
            case 2:
                a->appendNarratedString(_N("daisytype2"));
                break;
            case 3:
                a->appendNarratedString(_N("daisytype3"));
                break;
            case 4:
                a->appendNarratedString(_N("daisytype4"));
                break;
            case 5:
                a->appendNarratedString(_N("daisytype5"));
                a->appendNarratedString(_N("shortpause"));
                a->appendNarratedString(_N("playback may be lacking since book does not have complete audio"));
                break;
            case 6:
                a->appendNarratedString(_N("daisytype6"));
                a->appendNarratedString(_N("shortpause"));
                a->appendNarratedString(_N("playback may be lacking since book does not have complete audio"));
                break;
            case 7:
                a->appendNarratedString(_N("daisytype7"));
                a->appendNarratedString(_N("shortpause"));
                a->appendNarratedString(_N("playback may be lacking since book does not have complete audio"));
                break;
            }
        }
        a->appendNarratedString(_N("longpause"));
    }

    // SETINFO
    if (bookInfo->hasSetInfo && bookInfo->mMaxSet > 1)
    {
        NarratedNode* a = new NarratedNode(_N("set info"), ++childNumber);
        info->addNode(a);
        a->appendNarratedString(_N("shortpause"));
//        if(bookInfo->hasSetInfo && bookInfo->mMaxSet > 1) {
//            narrator->playItem(_N("series"), Speaker::ADD);
//            narrator->playItem(_N("material is part"), Speaker::ADD);
//            narrator->sayNumber(bookInfo->mCurrentSet);
//            narrator->playItem(_N("in a series with"), Speaker::ADD);
//            narrator->sayNumber(bookInfo->mMaxSet);
//            narrator->playItem(_N("parts"), Speaker::ADD);
//        }
//        break;
    }
}

// Static callback functions for daisyhandler, player and narrator
static bool PlayAudio_static(string uri, long long startms, long long stopms, void *data)
{
    DaisyNavi *dn = static_cast<DaisyNavi *>(data);
    return dn->playAudio(uri, startms, stopms);
}

bool DaisyNavi::onOpen(NaviEngine&)
{
    // connect to player slots
    if (not playerMsgCon.connected())
    {
        playerMsgCon = player->doOnPlayerMessage(boost::bind(&DaisyNavi::playerMessageSlot, this, _1));
    }
    if (not playerTimeCon.connected())
    {
        playerTimeCon = player->doOnPlayerTime(boost::bind(&DaisyNavi::playerTimeSlot, this, _1));
    }

    dh->setPlayFunction(PlayAudio_static, this);

    // If we are returning from context menu
    if (bContextMenuIsOpen)
    {
        bContextMenuIsOpen = false;
        // If the Narrator is speaking DaisyNavi wants to know when it ends
        Narrator::Instance()->setPushCommandFinished(true);
        narrator->play(_N("continuing"));
        narrator->playShortpause();
        // Publish book information again
        postBookData();
        // Announce current section
        lastpage = -1;
        lastsection = -1;
        return true;
    }
    bool hasLastmark = dh->setupBook();

    DaisyHandler::BookInfo *bookInfo = dh->getBookInfo();
    DaisyHandler::PosInfo *posInfo = dh->getPosInfo();
    postBookData();

    // Always enable section highlighting as it doesn't decrease performance significantly
    sectionIdxReportingEnabled = true;
    /*
     int pageCount = 0;
     if( bookInfo->hasPages ){
     pageCount = bookInfo->mFrontPages + bookInfo->mNormalPages + bookInfo->mSpecialPages;
     }
     sectionIdxReportingEnabled = ((bookInfo->mTocItems + pageCount) < 1000);
     */

    if (hasLastmark)
    {
        narrator->play(_N("continuing from last known position"));
        narrator->playShortpause();

        if (posInfo->hasCurrentTime)
        {
            if (posInfo->mPercentRead > 1)
            {
                narrator->setParameter("2", posInfo->mPercentRead);
                narrator->play(_N("{2} percent read"));
            }
            else
            {
                narrator->setParameter("1", posInfo->mPercentRead);
                narrator->play(_N("{1} percent read"));
            }
        }
    }
    else
    {
        if (not sectionIdxReportingEnabled)
        {
            narrator->play(_N("content contains too many navigation points, gui synchronization disabled"));
        }

        if (!narrator->isSpeaking())
            player->resume();
    }

    // We are now ready to handle NARRATORFINISHED COMMANDS
    Narrator::Instance()->setPushCommandFinished(true);

    DaisyNaviLevel level(dh->getNaviLevelStr());
    cq2::Command<DaisyNaviLevel> daisyLevel(level);
    daisyLevel();

    return true;
}

bool DaisyNavi::playerTimeSlot(Player::timeData td)
{
    const DaisyHandler::PosInfo *pi;
    const DaisyHandler::BookInfo *bi;
    pi = dh->getPosInfo();
    bi = dh->getBookInfo();
    if (td.current / 1000 != lastsecond)
    {

        if (td.current == -1 || pi->currentSmilms == -1 || td.duration == -1 || pi->totalSmilms == -1)
        {
        }
        else
        {
            if (bi->hasTime)
            {

                long bookCurrentms = td.current + pi->currentSmilms;
                long bookTotalms = bi->mTotalTime.tm_hour * 60 * 60 * 1000;
                bookTotalms += bi->mTotalTime.tm_min * 60 * 1000;
                bookTotalms += bi->mTotalTime.tm_sec * 1000;

                BookPositionInfo position(bookCurrentms, bookTotalms);
                cq2::Command<BookPositionInfo> info(position);
                info();
                lastsecond = td.current / 1000;
                pthread_mutex_lock(playerCallbackMutex);
                lastReportedPlayerPosition = bookCurrentms / 1000;
                pthread_mutex_unlock(playerCallbackMutex);
            }
        }
    }
    if (sectionIdxReportingEnabled)
    {
        if (pi->currentPageIdx != lastpage)
        {
            BookPageInfo page;
            if (bi->hasPages)
            {
                page.currentPageIdx = pi->currentPageIdx;
            }
            else
            {
                page.currentPageIdx = -1;
            }
            cq2::Command<BookPageInfo> info(page);
            info();
            lastpage = page.currentPageIdx;
        }

        if (pi->currentSectionIdx >= 0 && pi->currentSectionIdx != lastsection)
        {
            BookSectionInfo section(pi->currentSectionIdx - 1);
            cq2::Command<BookSectionInfo> info(section);
            info();
            lastsection = pi->currentSectionIdx;
        }
    }
    return true;
}

// command handlers

struct Handle_JumpToSecond: public cq2::Handler<JumpCommand<unsigned int> >
{
    Handle_JumpToSecond(DaisyNavi* dn) : dn_(dn) {}

private:
    DaisyNavi* dn_;

    void handle(JumpCommand<unsigned int> jump)
    {
        if (dn_->bPlaybackIsPaused)
        {
            dn_->bPlaybackIsPaused = false;
            Narrator::Instance()->setPushCommandFinished(true);
        }

        // pause player and enable narrator signals when finished
        Player::Instance()->pause();
        Narrator::Instance()->setPushCommandFinished(true);

        if (DaisyHandler::Instance()->getNaviLevel() <= DaisyHandler::TOPLEVEL)
        {
            LOG4CXX_WARN(daisyNaviLog, "Attempt to jump in top level, aborting jump");
            Narrator::Instance()->play(_N("jump to position failed"));
            Player::Instance()->reopen();
            return;
        }

        // perform the jump
        if (not DaisyHandler::Instance()->jumpToSecond(jump.target_))
        {
            LOG4CXX_ERROR(daisyNaviLog, "Jump to second '" << jump.target_ << "' failed");
            Narrator::Instance()->play(_N("jump to position failed"));
            Player::Instance()->reopen();
            return;
        }

        // jump was successful, resume playback
        // we could call Player::Instance()->resume() from here but the playback will resume
        // automatically when Narrator signals that it is done
    }
};

// command handlers end

// DaisyNavi constructor
DaisyNavi::DaisyNavi()
{
    // Disable NARRATORFINISHED COMMANDS while we are setting up the book
    Narrator::Instance()->setPushCommandFinished(false);

    narrator = Narrator::Instance();
    player = Player::Instance();
    dh = DaisyHandler::Instance();

    bPlaybackIsPaused = false;
    bContextMenuIsOpen = false;
    bBookIsOpen = false;
    bookmarkState = BOOKMARK_DEFAULT;

    // Setup mutex variable
    playerCallbackMutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(playerCallbackMutex, NULL);
    bPlayerReallySendEOS = false;

    DaisyNaviLevel level("TOPLEVEL");
    cq2::Command<DaisyNaviLevel> daisyLevel(level);
    daisyLevel();

    // command handler
    jumpHandler1 = new Handle_JumpToSecond(this);
    jumpHandler1->listen();
}

DaisyNavi::~DaisyNavi()
{
    // disconnect from player slots
    if (playerMsgCon.connected())
    {
        playerMsgCon.disconnect();
    }
    if (playerTimeCon.connected())
    {
        playerTimeCon.disconnect();
    }
}

bool DaisyNavi::playAudio(string filename, long long startms, long long stopms)
{
    if (!bBookIsOpen)
    {
        LOG4CXX_ERROR(daisyNaviLog, "Cannot play audio when book is closed");
        return false;
    }
    player->open(filename, startms, stopms);
    return true;
}

void DaisyNavi::setPlayerReallySendEOS(bool setting)
{
    pthread_mutex_lock(playerCallbackMutex);
    bPlayerReallySendEOS = setting;
    pthread_mutex_unlock(playerCallbackMutex);
}

bool DaisyNavi::getPlayerReallySendEOS()
{
    bool setting = false;
    pthread_mutex_lock(playerCallbackMutex);
    setting = bPlayerReallySendEOS;
    pthread_mutex_unlock(playerCallbackMutex);
    return setting;
}

bool DaisyNavi::playerMessageSlot(Player::playerMessage msg)
{
    switch (msg)
    {
    case Player::PLAYER_CONTINUE:
        LOG4CXX_INFO(daisyNaviLog, "CONTINUE callback");
        if (!bPlaybackIsPaused)
        {
            // This _used_ _to_ eventually result
            // in dh->nextPhrase() being called
            // from the kolibre_thread context
            // which continued the reader, but
            // this seems a bit silly since the
            // call passes through this class.
            // BUG: with the NaviEngine framework
            // COMMAND NEXT is not passed down
            // from ClientCore.cpp to DaisyNavi,
            // there is no reason why ClientCore.cpp
            // should know anything about a next
            // phrase command.
            cq2::Command<INTERNAL_COMMAND> c(COMMAND_NEXT);
            c();

            return true;
        }
        else
            return false;
        break;

    case Player::PLAYER_ATEOS:
        LOG4CXX_INFO(daisyNaviLog, "ATEOS callback");
        setPlayerReallySendEOS(true);
        usleep(1000000); // Wait for previous file to finish playing
        if (getPlayerReallySendEOS())
        {
            //player->stop();
            // This _used_ _to_ eventually result
            // in dh->nextPhrase() being called
            // from the kolibre_thread context
            // which continued the reader, but
            // this seems a bit silly since the
            // call passes through this class.
            // BUG: with the NaviEngine framework
            // COMMAND NEXT is not passed down
            // from ClientCore.cpp to DaisyNavi,
            // there is no reason why ClientCore.cpp
            // should know anything about a next
            // phrase command.
            cq2::Command<INTERNAL_COMMAND> c(COMMAND_NEXT);
            c();
        }
        else
        {
            LOG4CXX_WARN(daisyNaviLog, "ATEOS not sending COMMAND_NEXT since some player event happened while waiting for stream to finish playing");
        }
        return true;
        break;

    case Player::PLAYER_BUFFERING:
    {
        LOG4CXX_INFO(daisyNaviLog, "BUFFER callback");
        usleep(500000);
        narrator->playWait();
        return false;
        break;
    }

    case Player::PLAYER_ERROR:
    {
        LOG4CXX_INFO(daisyNaviLog, "ERROR callback");
        usleep(500000);
        narrator->play(_N("error loading data"));
        ErrorMessage error(NETWORK, "Error streaming data");
        cq2::Command<ErrorMessage> message(error);
        message();
        closeBook();
    }
        return false;
        break;
    }
    return false;
}

bool DaisyNavi::isOpen()
{
    return bBookIsOpen;
}

bool DaisyNavi::open(const string &uri)
{

    string newuri = uri;
    // Extract protocol, servername and path from url
    string protocol = "";
    string username;
    string password;
    string server = "";
    string path = "";
    string tmp;
    bUserAtEndOfBook = false;

    string::size_type pos = newuri.find("//");
    if (pos != string::npos)
    {
        protocol = newuri.substr(0, pos + 2);
        tmp = newuri.substr(pos + 2);
        pos = tmp.find("/");
        if (pos != string::npos)
        {
            server = tmp.substr(0, pos);
            path = tmp.substr(pos);

            if (username != "" && password != "")
            {
                newuri = protocol + username + ":" + password + "@" + server + path;
            }
        }
    }

    LOG4CXX_DEBUG(daisyNaviLog, "opening book " << uri);

    if (dh->openBook(newuri) == true)
    {
        int waitCounter = 0;
        while (dh->getState() == DaisyHandler::HANDLER_OPENING)
        {
            LOG4CXX_DEBUG(daisyNaviLog, "state == DaisyHandler::HANDLER_OPENING");
            usleep(100000);
            // when 3 seconds has passed, play wait jingle
            if (waitCounter++ == 30)
            {
                Narrator::Instance()->playWait();
                waitCounter = 0;
            }
        }

        if (dh->getState() != DaisyHandler::HANDLER_OPEN)
        {
            LOG4CXX_DEBUG(daisyNaviLog, "state != DaisyHandler::HANDLER_OPEN");
            narrator->play(_N("content error"));
            amis::AmisError err = dh->getLastError();
            ErrorMessage error(NETWORK, err.getMessage());
            cq2::Command<ErrorMessage> message(error);
            message();
            usleep(100000);
            while (narrator->isSpeaking())
                usleep(20000);
            return false;
        }
        LOG4CXX_DEBUG(daisyNaviLog, "state == DaisyHandler::HANDLER_OPEN");

        bBookIsOpen = true;

        return true;
    }
    else
    {
        LOG4CXX_WARN(daisyNaviLog, "failed to load data");
        narrator->play(_N("error loading data"));
        amis::AmisError err = dh->getLastError();
        ErrorMessage error(NETWORK, err.getMessage());
        cq2::Command<ErrorMessage> message(error);
        message();
        usleep(100000);
        while (narrator->isSpeaking())
            usleep(20000);
        return false;
    }

    return true;
}

bool DaisyNavi::closeBook()
{
    bBookIsOpen = false;
    LOG4CXX_DEBUG(daisyNaviLog, "closing book");
    player->stop();
    dh->closeBook();
    DaisyNaviLevel level("TOPLEVEL");
    cq2::Command<DaisyNaviLevel> daisyLevel(level);
    daisyLevel();
    return true;
}

void DaisyNavi::sayLevel(DaisyHandler::NaviLevel level, bool verbose)
{
    if (player->isPlaying())
    {
        player->pause();
    }

    if (verbose)
    {
        narrator->play(_N("navilevel"));
        narrator->playShortpause();
    }
    switch (level)
    {
    case DaisyHandler::TOPLEVEL:
        narrator->play(_N("publication list"));
        break;
    case DaisyHandler::BEGEND:
        narrator->play(_N("begend"));
        break;
    case DaisyHandler::BOOKMARK:
        narrator->play(_N("bookmark"));
        break;
    case DaisyHandler::HISTORY:
        narrator->play(_N("navi history"));
        break;
    case DaisyHandler::H1:
        narrator->play(_N("level"));
        narrator->play(1);
        break;
    case DaisyHandler::H2:
        narrator->play(_N("level"));
        narrator->play(2);
        break;
    case DaisyHandler::H3:
        narrator->play(_N("level"));
        narrator->play(3);
        break;
    case DaisyHandler::H4:
        narrator->play(_N("level"));
        narrator->play(4);
        break;
    case DaisyHandler::H5:
        narrator->play(_N("level"));
        narrator->play(5);
        break;
    case DaisyHandler::H6:
        narrator->play(_N("level"));
        narrator->play(6);
        break;
    case DaisyHandler::PAGE:
        narrator->play(_N("page"));
        break;
    case DaisyHandler::PHRASE:
        narrator->play(_N("phrase"));
        break;
    }
}

bool DaisyNavi::process(NaviEngine& navi, int command, void* data)
{
    LOG4CXX_DEBUG(daisyNaviLog, "Processing command: " << command);
    bool amisSuccess = true;
    unsigned int totalTime;
    setPlayerReallySendEOS(false);
    DaisyHandler::BookInfo *bookInfo;

    // If we are pausing, any key should continue playback
    if (bPlaybackIsPaused && command != COMMAND_INFO && command != COMMAND_NARRATORFINISHED)
    {
        command = COMMAND_PAUSE;
    }
    // Check the bookmarkstate, if non-valid key do timeout
    if (bookmarkState != BOOKMARK_DEFAULT)
        if (command != COMMAND_DOWN && command != COMMAND_BOOKMARK && command != COMMAND_NARRATORFINISHED)
        {
            bookmarkState = BOOKMARK_TIMEOUT;
            command = COMMAND_BOOKMARK;
        }

    //14.11.2011 - This will never happen!? most likely we check for wrong internal state. perhaps we were interested in getRealState?
    if (player->getState() == BUFFERING)
    {
        LOG4CXX_WARN(daisyNaviLog, "Player is still buffering last clip");
        //player->stop();
    }

    switch (command)
    {
    case COMMAND_NARRATORFINISHED:
        if (!bBookIsOpen)
        {
            LOG4CXX_WARN(daisyNaviLog, "Narrator finished but cannot resume player since book is not open");
            return false;
        }

        if (bPlaybackIsPaused)
        {
            LOG4CXX_WARN(daisyNaviLog, "Narrator finished but wont resume player in PAUSE mode");
            return false;
        }

        if (bookmarkState != BOOKMARK_DEFAULT)
        {
            LOG4CXX_WARN(daisyNaviLog, "Narrator finished but wont resume player in BOOKMARK mode");
            return false;
        }

        if (bContextMenuIsOpen)
        {
            LOG4CXX_WARN(daisyNaviLog, "Narrator finished but wont resume since context menu is open");
            return false;
        }

        if (bUserAtEndOfBook)
        {
            LOG4CXX_WARN(daisyNaviLog, "Narrator finished but wont resume since player reached the end of book");
            return false;
        }

        LOG4CXX_INFO(daisyNaviLog, "Narrator finished, resuming playback");
        player->resume();
        break;

    case COMMAND_UP:
        LOG4CXX_INFO(daisyNaviLog, "COMMAND_UP received");
        if (!bBookIsOpen)
        {
            LOG4CXX_WARN(daisyNaviLog, "Book is not open, no need to process COMMAND_UP");
            return false;
        }
        player->pause();
        dh->increaseNaviLevel();
        // If we went too high up close the book and return control to navi
        {
            DaisyNaviLevel level(dh->getNaviLevelStr());
            cq2::Command<DaisyNaviLevel> daisyLevel(level);
            daisyLevel();
        }
        if (dh->getNaviLevel() == DaisyHandler::TOPLEVEL)
        {
            LOG4CXX_INFO(daisyNaviLog, "At top level, closing book");
            closeBook();
            return false;
        }

        dh->printNaviLevel();
        sayLevel(dh->getNaviLevel(), true);
        break;

    case COMMAND_DOWN:
        LOG4CXX_INFO(daisyNaviLog, "COMMAND_DOWN received");
        // Check if the user wants to confirm a bookmark action
        if (bookmarkState != BOOKMARK_DEFAULT)
        {
            switch (bookmarkState)
            {
            case BOOKMARK_ADD:
            {
                int num = dh->getNextBookmarkId();
                amisSuccess = dh->addBookmark();
                if (amisSuccess)
                {
                    LOG4CXX_INFO(daisyNaviLog, "Bookmark added");
                    narrator->setParameter("1", num);
                    narrator->play(_N("adding bookmark no. {1}"));
                    narrator->playLongpause();
                    narrator->play(_N("continuing"));
                    narrator->playShortpause();
                }
                else
                {
                    LOG4CXX_ERROR(daisyNaviLog, "Failed to add bookmark");
                }

                bookmarkState = BOOKMARK_DEFAULT;
                break;
            }

            case BOOKMARK_DELETE:
            {
                amisSuccess = false;
                int num = dh->getCurrentBookmarkId();
                if (num != -1)
                {
                    amisSuccess = dh->deleteCurrentBookmark();
                    if (amisSuccess)
                    {
                        LOG4CXX_INFO(daisyNaviLog, "Bookmark deleted");
                        narrator->setParameter("1", num);
                        narrator->play(_N("deleting bookmark no. {1}"));
                        narrator->playLongpause();
                        narrator->play(_N("continuing"));
                        narrator->playShortpause();
                    }
                    else
                    {
                        LOG4CXX_ERROR(daisyNaviLog, "Failed to delete bookmark");
                    }
                }

                bookmarkState = BOOKMARK_DEFAULT;
                break;
            }

            case BOOKMARK_DELETEALL:
                amisSuccess = false;
                if (dh->getNumberOfBookmarks() > 0)
                {
                    amisSuccess = dh->deleteAllBookmarks();
                    if (amisSuccess)
                    {
                        LOG4CXX_INFO(daisyNaviLog, "All bookmarks deleted");
                        narrator->play(_N("deleting all bookmarks"));
                        narrator->playLongpause();
                        narrator->play(_N("continuing"));
                        narrator->playShortpause();
                    }
                    else
                    {
                        LOG4CXX_ERROR(daisyNaviLog, "Failed to delete all bookmarks");
                    }
                }

                bookmarkState = BOOKMARK_DEFAULT;
                break;

            case BOOKMARK_INFO:
                narrator->playLongpause();
                narrator->play(_N("continuing"));
                narrator->playShortpause();
                bookmarkState = BOOKMARK_DEFAULT;
                break;

            default:
                bookmarkState = BOOKMARK_DEFAULT;
                break;
            }

            // If we've deleted all the bookmarks, and active level is BOOKMARKS, decrease navilevel
            if (dh->getNaviLevel() == DaisyHandler::BOOKMARK && dh->getNumberOfBookmarks() == 0)
            {
                dh->decreaseNaviLevel();
                dh->printNaviLevel();
                sayLevel(dh->getNaviLevel(), true);
            }

            // break from case COMMAND_DOWN:
            break;
        }

        player->pause();
        if (dh->getNaviLevel() != DaisyHandler::PHRASE)
            dh->decreaseNaviLevel();
        dh->printNaviLevel();
        sayLevel(dh->getNaviLevel(), true);
        {
            DaisyNaviLevel level(dh->getNaviLevelStr());
            cq2::Command<DaisyNaviLevel> daisyLevel(level);
            daisyLevel();
        }
        break;

    case COMMAND_RIGHT:
        LOG4CXX_INFO(daisyNaviLog, "COMMAND_RIGHT received");
        switch (dh->getNaviLevel())
        {
        case DaisyHandler::BEGEND:
            player->pause();
            LOG4CXX_INFO(daisyNaviLog, "Going to end of book");
            amisSuccess = dh->lastSection();
            LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
            if (amisSuccess)
            {
                narrator->play(_N("end of content"));
            }
            break;

        case DaisyHandler::BOOKMARK:
            player->pause();
            LOG4CXX_INFO(daisyNaviLog, "Going to next bookmark");
            amisSuccess = dh->nextBookmark();
            LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
            if (amisSuccess)
            {
                narrator->setParameter("1", dh->getCurrentBookmarkId());
                narrator->play(_N("loading bookmark no. {1}"));
            }
            break;
        case DaisyHandler::HISTORY:
            player->pause();
            LOG4CXX_INFO(daisyNaviLog, "Going to next history item");
            amisSuccess = dh->nextHistory();
            LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
            if (amisSuccess)
            {
                narrator->play(_N("next"));
                narrator->play(_N("navi history"));
            }
            else
            {
                narrator->play(_N("end of navi history"));
                amisSuccess = true;
            }
            break;

        case DaisyHandler::H1:
        case DaisyHandler::H2:
        case DaisyHandler::H3:
        case DaisyHandler::H4:
        case DaisyHandler::H5:
        case DaisyHandler::H6:
            player->stop();
            narrator->play(_N("next"));
            sayLevel(dh->getNaviLevel());
            LOG4CXX_INFO(daisyNaviLog, "Going to next heading");
            amisSuccess = dh->nextSection();
            LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
            break;

        case DaisyHandler::PAGE:
            player->pause();
            narrator->stop();
            LOG4CXX_INFO(daisyNaviLog, "Going to next page");
            amisSuccess = dh->nextPage();
            LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
            if (amisSuccess)
            {
                narrator->play(_N("next"));
                sayLevel(dh->getNaviLevel());
                if (dh->getCustomTestState("pagenumber") == 1 && dh->getCurrentPage() != "")
                {
                    stringstream ss(dh->getCurrentPage());
                    int n;
                    char c;
                    if (ss >> n && !ss.get(c))
                        narrator->play(n);
                    else
                        LOG4CXX_WARN(daisyNaviLog, "Could not convert page (" << dh->getCurrentPage() << ") to integer");
                }
            }
            break;

        case DaisyHandler::PHRASE:
            player->pause();
            LOG4CXX_INFO(daisyNaviLog, "Going to next phrase");
            amisSuccess = dh->nextPhrase();
            LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
            if (amisSuccess)
            {
                player->resume();
            }
            break;
        }
        break;

    case COMMAND_LEFT:
        LOG4CXX_INFO(daisyNaviLog, "COMMAND_LEFT received");
        switch (dh->getNaviLevel())
        {
        case DaisyHandler::BEGEND:
            player->pause();
            LOG4CXX_INFO(daisyNaviLog, "Going to beginning of book");
            amisSuccess = dh->firstSection();
            LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
            if (amisSuccess)
            {
                narrator->play(_N("start of content"));
            }
            break;

        case DaisyHandler::BOOKMARK:
            player->pause();
            LOG4CXX_INFO(daisyNaviLog, "Going to previous bookmark");
            amisSuccess = dh->previousBookmark();
            LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
            if (amisSuccess)
            {
                narrator->setParameter("1", dh->getCurrentBookmarkId());
                narrator->play(_N("loading bookmark no. {1}"));
            }
            break;

        case DaisyHandler::HISTORY:
            player->pause();
            LOG4CXX_INFO(daisyNaviLog, "Going to previous history item");
            amisSuccess = dh->previousHistory();
            LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
            if (amisSuccess)
            {
                narrator->play(_N("previous"));
                narrator->play(_N("navi history"));
            }
            else
            {
                narrator->play(_N("start of navi history"));
                amisSuccess = true;
            }
            break;

        case DaisyHandler::H1:
        case DaisyHandler::H2:
        case DaisyHandler::H3:
        case DaisyHandler::H4:
        case DaisyHandler::H5:
        case DaisyHandler::H6:
            player->stop();
            narrator->play(_N("previous"));
            sayLevel(dh->getNaviLevel());

            LOG4CXX_INFO(daisyNaviLog, "Going to previous header");
            amisSuccess = dh->previousSection();
            LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
            break;

        case DaisyHandler::PAGE:
            player->pause();
            LOG4CXX_INFO(daisyNaviLog, "Going to previous page");
            amisSuccess = dh->previousPage();
            LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
            if (amisSuccess)
            {
                narrator->play(_N("previous"));
                sayLevel(dh->getNaviLevel());
                if (dh->getCustomTestState("pagenumber") == 1 && dh->getCurrentPage() != "")
                {
                    istringstream ss(dh->getCurrentPage());
                    int n;
                    char c;
                    if (ss >> n && !ss.get(c))
                        narrator->play(n);
                    else
                        LOG4CXX_WARN(daisyNaviLog, "Could not convert page (" << dh->getCurrentPage() << ") to integer");
                }
            }
            break;

        case DaisyHandler::PHRASE:
            player->pause();
            LOG4CXX_INFO(daisyNaviLog, "Going to previous page");
            amisSuccess = dh->previousPhrase();
            LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
            if (amisSuccess)
            {
                //narrator->play(_N("previous"));
                //sayLevel(level, Narrator::APPEND);
                player->resume();
            }
            break;

        }
        break;

    case COMMAND_NEXT:
        LOG4CXX_INFO(daisyNaviLog, "COMMAND_NEXT received");
        LOG4CXX_INFO(daisyNaviLog, "Going to next phrase");
        amisSuccess = dh->nextPhrase();
        LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
        if (amisSuccess && !player->isPlaying() && !narrator->isSpeaking())
            player->resume();
        break;

    case COMMAND_PAUSE:
        LOG4CXX_INFO(daisyNaviLog, "COMMAND_PAUSE received");
        if (not bPlaybackIsPaused)
        {
            LOG4CXX_INFO(daisyNaviLog, "pausing playback");
            bPlaybackIsPaused = true;
            player->pause();
            Narrator::Instance()->setPushCommandFinished(false);
            narrator->play(_N("pausing"));
        }
        else
        {
            LOG4CXX_INFO(daisyNaviLog, "resuming playback");
            bPlaybackIsPaused = false;
            player->reopen();
            Narrator::Instance()->setPushCommandFinished(true);
            narrator->play(_N("continuing"));
            narrator->playShortpause();
        }
        break;

    case COMMAND_LAST:
        LOG4CXX_INFO(daisyNaviLog, "COMMAND_LAST received");
        LOG4CXX_INFO(daisyNaviLog, "Going to last history item");
        amisSuccess = dh->loadLastHistory();
        LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
        amisSuccess = true;
        break;

    case COMMAND_BACK:
        LOG4CXX_INFO(daisyNaviLog, "COMMAND_BACK received");
        LOG4CXX_INFO(daisyNaviLog, "Going to previous history item");
        player->pause();
        amisSuccess = dh->previousHistory();
        LOG4CXX_INFO(daisyNaviLog, "operation" << (amisSuccess == true ? " was successful" : " failed"));
        if (amisSuccess)
            narrator->play(_N("previous navi history element"));
        else
        {
            narrator->play(_N("start of navi history"));
            amisSuccess = true;
        }
        break;

    case COMMAND_BOOKMARK:
        LOG4CXX_INFO(daisyNaviLog, "COMMAND_BOOKMARK received");

        if (bookmarkState == BOOKMARK_TIMEOUT)
        {
            bookmarkState = BOOKMARK_DEFAULT;
            narrator->play(_N("continuing"));
            narrator->playShortpause();
            break;
        }

        player->pause();
        narrator->stop();
        // Change the state depending on the current state
        switch (bookmarkState)
        {
        case BOOKMARK_DEFAULT:
            bookmarkState = BOOKMARK_ADD;
            break;

        case BOOKMARK_ADD:
            bookmarkState = BOOKMARK_INFO;
            break;

        case BOOKMARK_INFO:
            if (dh->getNumberOfBookmarks() > 0 && // Check that we have bookmarks
            dh->getCurrentBookmarkId() != -1)
            { // Check that we have selected a bookmark
                bookmarkState = BOOKMARK_DELETE;
            }
            else
            {
                bookmarkState = BOOKMARK_ADD;
            }
            break;

        case BOOKMARK_DELETE:
            if (dh->getNumberOfBookmarks() > 0)
            { // Check that we have bookmarks
                bookmarkState = BOOKMARK_DELETEALL;
            }
            else
            {
                bookmarkState = BOOKMARK_ADD;
            }
            break;

        case BOOKMARK_DELETEALL:
            bookmarkState = BOOKMARK_ADD;
            break;
        }

        // Tell the user what's going on
        switch (bookmarkState)
        {
        case BOOKMARK_ADD:
            LOG4CXX_INFO(daisyNaviLog, "Confirm addition of bookmark");
            narrator->play(_N("add new bookmark"));
            narrator->play(_N("confirm using down arrow"));
            break;

        case BOOKMARK_INFO:
        {
            LOG4CXX_INFO(daisyNaviLog, "Information about bookmarks");

            int num = dh->getNumberOfBookmarks();
            if (num == 0)
            {
                narrator->play(_N("content has no bookmarks"));
            }
            else if (num == 1)
            {
                narrator->setParameter("1", num);
                narrator->play(_N("content has {1} bookmark"));
            }
            else
            {
                narrator->setParameter("2", num);
                narrator->play(_N("content has {2} bookmarks"));
            }

            num = dh->getCurrentBookmarkId();
            if (num != -1)
            {
                narrator->setParameter("1", num);
                narrator->play(_N("current bookmark is no. {1}"));
            }
            break;
        }

        case BOOKMARK_DELETE:
            LOG4CXX_INFO(daisyNaviLog, "Confirm deletion of bookmark");
            narrator->setParameter("1", dh->getCurrentBookmarkId());
            narrator->play(_N("delete bookmark no. {1}"));
            narrator->play(_N("confirm using down arrow"));
            break;

        case BOOKMARK_DELETEALL:
            LOG4CXX_INFO(daisyNaviLog, "Confirm deletion of all bookmarks");
            narrator->play(_N("delete all bookmarks"));
            narrator->play(_N("confirm using down arrow"));
            break;
        }
        break;
    case COMMAND_OPEN_BOOKINFO:
    {
        BookInfoNode* infoNode = new BookInfoNode(_N("content info"), _N("loading content info"));
        buildInfoNode(infoNode);
        if (navi.openMenu(infoNode, false))
        {
            bContextMenuIsOpen = true;
            Narrator::Instance()->setPushCommandFinished(false); // While menu is open don't send NARRATORFINISHED COMMANDS TO THE QUEUE
        }
        break;
    }
    case COMMAND_OPEN_MENU_GOTOTIMENODE:
    {
        bookInfo = dh->getBookInfo();
        totalTime = bookInfo->mTotalTime.tm_hour * 60 * 60 + bookInfo->mTotalTime.tm_min * 60 + bookInfo->mTotalTime.tm_sec;
        AnyNode* timeJump = new GotoTimeNode(totalTime, this);
        if (navi.openMenu(timeJump, false))
        {
            bContextMenuIsOpen = true;
            Narrator::Instance()->setPushCommandFinished(false); // While menu is open don't send NARRATORFINISHED COMMANDS TO THE QUEUE
        }
        break;
    }
    case COMMAND_OPEN_MENU_GOTOPERCENTNODE:
    {
        bookInfo = dh->getBookInfo();
        totalTime = bookInfo->mTotalTime.tm_hour * 60 * 60 + bookInfo->mTotalTime.tm_min * 60 + bookInfo->mTotalTime.tm_sec;
        AnyNode* percentJump = new GotoPercentNode(totalTime, this);
        if (navi.openMenu(percentJump, false))
        {
            bContextMenuIsOpen = true;
            Narrator::Instance()->setPushCommandFinished(false); // While menu is open don't send NARRATORFINISHED COMMANDS TO THE QUEUE
        }
        break;
    }
    case COMMAND_OPEN_MENU_GOTOPAGENODE:
    {
        bookInfo = dh->getBookInfo();
        totalTime = bookInfo->mTotalTime.tm_hour * 60 * 60 + bookInfo->mTotalTime.tm_min * 60 + bookInfo->mTotalTime.tm_sec;
        if (bookInfo->hasPages && bookInfo->mNormalPages > 0)
        {
            // Sum the total of pages.
            int pageCount = bookInfo->mNormalPages + bookInfo->mSpecialPages + bookInfo->mFrontPages;
            AnyNode* pageJump = new GotoPageNode(pageCount, this);
            if (navi.openMenu(pageJump, false))
            {
                bContextMenuIsOpen = true;
                Narrator::Instance()->setPushCommandFinished(false); // While menu is open don't send NARRATORFINISHED COMMANDS TO THE QUEUE
            }
        }
        break;
    }
    case COMMAND_INFO: // Will be sent if we are quiet for too long
        LOG4CXX_INFO(daisyNaviLog, "COMMAND_INFO received");
        if (bPlaybackIsPaused)
        {
            LOG4CXX_INFO(daisyNaviLog, "Playback is paused, doing nothing");
        }
        else if (bUserAtEndOfBook)
        {
            LOG4CXX_INFO(daisyNaviLog, "At end of book, closing book");
            closeBook();
        }
        else if (bookmarkState != BOOKMARK_DEFAULT)
        {
            LOG4CXX_INFO(daisyNaviLog, "resuming playback");
            bookmarkState = BOOKMARK_DEFAULT;
            narrator->play(_N("continuing"));
            narrator->playShortpause();
        }
        break;
    default:
        LOG4CXX_INFO(daisyNaviLog, "no action for this command");
        break;
    }

    // Check return code of executed command
    if (amisSuccess == false)
    {
        amis::AmisError err = dh->getLastError();
        switch (err.getCode())
        {
        case amis::AT_END:
            LOG4CXX_ERROR(daisyNaviLog, "Jump failed with error AT_END");
            narrator->play(_N("end of content"));
            player->stop();
            bUserAtEndOfBook = true;
            break;

        case amis::AT_BEGINNING:
            LOG4CXX_ERROR(daisyNaviLog, "Jump failed with error AT_BEGINNING");
            narrator->play(_N("start of content"));
            break;

        case amis::NOT_FOUND:
            LOG4CXX_ERROR(daisyNaviLog, "Jump failed with error NOT_FOUND");
            narrator->play(_N("error loading data"));
            break;

        case amis::UNDEFINED_ERROR:
            LOG4CXX_ERROR(daisyNaviLog, "Jump failed with error UNDEFINED_ERROR");
            narrator->play(_N("content error"));
            break;

        case amis::NOT_SUPPORTED:
            LOG4CXX_ERROR(daisyNaviLog, "Jump failed with error NOT_SUPPORTED");
            narrator->play(_N("content error"));
            break;

        case amis::PARSE_ERROR:
            LOG4CXX_ERROR(daisyNaviLog, "Jump failed with error PARSE_ERROR");
            narrator->play(_N("content error"));
            break;

        case amis::NOT_INITIALIZED:
            LOG4CXX_ERROR(daisyNaviLog, "Jump failed with error NOT_INITIALIZED");
            narrator->play(_N("content error"));
            break;

        case amis::OK:
            LOG4CXX_WARN(daisyNaviLog, "Jump failed with error OK");
            break;
        default:
            LOG4CXX_ERROR(daisyNaviLog, "Jump failed with no error set");
            break;
        }
    }
    else
    {
        // unset bUserAtEndOfBook on amisSuccess except for COMMAND_UP and COMMAND_DOWN
        switch (command)
        {
        case COMMAND_UP:
        case COMMAND_DOWN:
            break;
        default:
            bUserAtEndOfBook = false;
            break;
        }
    }

    return true;
}

void DaisyNavi::postBookData()
{
    ClientCore::BookData data;

    // navigation points
    DaisyHandler::NavPoints* navPoints = dh->getNavPoints();

    // pages
    for (int i = 0; i < navPoints->pages.size(); i++)
    {
        if (navPoints->pages[i].text.empty())
        {
            navPoints->pages[i].text = _("no page label");
        }
        ClientCore::BookData::Page page(navPoints->pages[i].id,
                navPoints->pages[i].text,
                navPoints->pages[i].playOrder);
        data.pages.push_back(page);
    }

    // sections
    for (int i = 0; i < navPoints->sections.size(); i++)
    {
        if (navPoints->sections[i].text.empty())
        {
            navPoints->sections[i].text = _("no section label");
        }
        ClientCore::BookData::Section section(navPoints->sections[i].id,
                navPoints->sections[i].text,
                navPoints->sections[i].playOrder,
                navPoints->sections[i].level);
        data.sections.push_back(section);
    }

    cq2::Command<ClientCore::BookData> bookData(data);
    bookData();
}

string DaisyNavi::getPageId(int pageNumber)
{
    return dh->getPageId(pageNumber);
}

string DaisyNavi::getPageLabel(int pageNumber)
{
    return dh->getPageLabel(pageNumber);
}

int DaisyNavi::getCurrentTime()
{
    DaisyHandler::BookInfo *bookInfo = dh->getBookInfo();
    if (bookInfo->hasTime)
    {
        pthread_mutex_lock(playerCallbackMutex);
        int pos = lastReportedPlayerPosition;
        pthread_mutex_unlock(playerCallbackMutex);
        LOG4CXX_INFO(daisyNaviLog, "current time is " << pos);
        return pos;
    }
    return 0;
}

int DaisyNavi::getCurrentPageIdx()
{
    if (dh->getBookInfo()->hasPages)
        return dh->getPosInfo()->currentPageIdx;
    return 0;
}

int DaisyNavi::getCurrentPercent()
{
    DaisyHandler::BookInfo *bookInfo = dh->getBookInfo();
    if (bookInfo->hasTime)
    {
        int totalTime = bookInfo->mTotalTime.tm_hour * 3600 + bookInfo->mTotalTime.tm_min * 60 + bookInfo->mTotalTime.tm_sec;
        return getCurrentTime() * 100 / totalTime;
    }
    return 0;
}
