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

#include "RootNode.h"
#include "DaisyOnlineNode.h"
#include "FileSystemNode.h"
#include "MP3Node.h"
#include "Defines.h"
#include "config.h"
#include "CommandQueue2/CommandQueue.h"
#include "Commands/InternalCommands.h"
#include "MediaSourceManager.h"
#include "Settings/Settings.h"
#include "Utils.h"

#include <Narrator.h>
#include <NaviEngine.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr rootNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.rootnode"));

using namespace naviengine;

RootNode::RootNode(const std::string useragent)
{
    LOG4CXX_TRACE(rootNodeLog, "Constructor");
    userAgent_ = useragent;
    name_ = "Root";
    openFirstChild_ = true;
}

RootNode::~RootNode()
{
    LOG4CXX_TRACE(rootNodeLog, "Destructor");
}

// NaviEngine functions

bool RootNode::next(NaviEngine& navi)
{
    bool ret = MenuNode::next(navi);
    currentChild_ = navi.getCurrentChoice();
    announceSelection();
    return ret;
}

bool RootNode::prev(NaviEngine& navi)
{
    bool ret = MenuNode::prev(navi);
    currentChild_ = navi.getCurrentChoice();
    announceSelection();
    return ret;
}

bool RootNode::menu(NaviEngine& navi)
{
    return navi.openMenu(navi.buildContextMenu());
}

bool RootNode::up(NaviEngine& navi)
{
    bool ret = MenuNode::up(navi);
    return ret;
}

bool RootNode::onOpen(NaviEngine& navi)
{
    navi.setCurrentChoice(NULL);
    clearNodes();
    navilist_.items.clear();

    // Create sources defined in MediaSourceManager
    LOG4CXX_INFO(rootNodeLog, "Creating children for root");

    // Split userAgent into model/version
    std::string model = "";
    std::string version = "";
    std::string::size_type slashpos = userAgent_.find('/');
    if (slashpos != std::string::npos)
    {
        model = userAgent_.substr(0, slashpos);
        version = userAgent_.substr(slashpos + 1);
    }

    int daisyOnlineServices = MediaSourceManager::Instance()->getDaisyOnlineServices();
    for (int i = 0; i < daisyOnlineServices; i++)
    {
        std::string name = MediaSourceManager::Instance()->getDOSname(i);
        std::string url = MediaSourceManager::Instance()->getDOSurl(i);
        std::string username = MediaSourceManager::Instance()->getDOSusername(i);
        std::string password = MediaSourceManager::Instance()->getDOSpassword(i);

        // Create a DaisyOnlineNode
        DaisyOnlineNode* daisyOnlineNode = new DaisyOnlineNode(name, url, username, password, userAgent_, openFirstChild_);
        if (daisyOnlineNode->good())
        {
            if (not model.empty() && not version.empty())
            {
                daisyOnlineNode->setModel(model);
                daisyOnlineNode->setVersion(version);
            }

            daisyOnlineNode->setLanguage(Settings::Instance()->read<std::string>("language", "en"));
            LOG4CXX_INFO(rootNodeLog, "Adding DaisyOnlineService '" << name << "'");
            addNode(daisyOnlineNode);

            // create a NaviListItem and store it in list for the NaviList signal
            NaviListItem item(daisyOnlineNode->uri_, name);
            navilist_.items.push_back(item);
        }
        else
        {
            LOG4CXX_ERROR(rootNodeLog, "DaisyOnlineNode failed to initialize");
        }
    }

    int fileSystemPaths = MediaSourceManager::Instance()->getFileSystemPaths();
    for (int i = 0; i < fileSystemPaths; i++)
    {
        std::string name = MediaSourceManager::Instance()->getFSPname(i);
        std::string path = MediaSourceManager::Instance()->getFSPpath(i);

        if (Utils::isDir(path))
        {
            LOG4CXX_INFO(rootNodeLog, "Adding FileSystemNode '" << name << "'");
            FileSystemNode *fileSystemNode = new FileSystemNode(name, path, openFirstChild_);
            addNode(fileSystemNode);

            // create a NaviListItem and store it in list for the NaviList signal
            NaviListItem item(fileSystemNode->uri_, name);
            navilist_.items.push_back(item);
        }
    }

//------------------------------------------------------------------------------------------------------------------------------
    // Add mp3 path
    int mp3Paths = MediaSourceManager::Instance()->getMP3Paths();
    for (int i = 0; i < mp3Paths; i++)
    {
        std::string name = MediaSourceManager::Instance()->getMP3Pname(i);
        std::string path = MediaSourceManager::Instance()->getMP3Ppath(i);

        if (Utils::isDir(path))
        {
            LOG4CXX_INFO(rootNodeLog, "Adding MP3Node '" << name << "'");
            MP3Node *mp3Node = new MP3Node(name, path, openFirstChild_);
            addNode(mp3Node);

            // create a NaviListItem and store it in list for the NaviList signal
            NaviListItem item(mp3Node->uri_, name);
            navilist_.items.push_back(item);
        }
    }
    // End adding mp3
//------------------------------------------------------------------------------------------------------------------------------

    if (navi.getCurrentChoice() == NULL && numberOfChildren() > 0)
    {
        navi.setCurrentChoice(firstChild());
        currentChild_ = firstChild();
    }

    currentChild_ = navi.getCurrentChoice();
    announce();

    bool autoPlay = Settings::Instance()->read<bool>("autoplay", true);
    if (autoPlay && openFirstChild_)
    {
        narratorDoneConnection = Narrator::Instance()->connectAudioFinished(boost::bind(&RootNode::onNarratorDone, this));
        Narrator::Instance()->setPushCommandFinished(true);
    }

    return true;
}

bool RootNode::process(NaviEngine& navi, int command, void* data)
{
    return false;
}
bool RootNode::narrateInfo()
{
    const bool isSelfNarrated = true;
    Narrator::Instance()->play(_N("choose option using left and right arrows, open using play button"));
    Narrator::Instance()->playLongpause();
    announceSelection();
    return isSelfNarrated;
}

bool RootNode::onNarrate()
{
    const bool isSelfNarrated = true;
    return isSelfNarrated;
}

bool RootNode::onRender()
{
    const bool isSelfRendered = true;
    return isSelfRendered;
}

bool RootNode::abort()
{
    if (openFirstChild_)
    {
        openFirstChild_ = false;
        Narrator::Instance()->setPushCommandFinished(false);
        narratorDoneConnection.disconnect();

        // abort auto open in children
        if (numberOfChildren() >= 1)
        {
            AnyNode* current = firstChild();
            for (int i=0; i<numberOfChildren(); i++)
            {
                current->abort();
                current = current->next_;
            }
        }
    }

    return true;
}

void RootNode::onNarratorDone()
{
    bool autoPlay = Settings::Instance()->read<bool>("autoplay", true);
    if (autoPlay && openFirstChild_ && numberOfChildren() >= 1)
    {
        openFirstChild_ = false;
        Narrator::Instance()->setPushCommandFinished(false);
        narratorDoneConnection.disconnect();

        LOG4CXX_INFO(rootNodeLog, "auto open first child");
        cq2::Command<INTERNAL_COMMAND> c(COMMAND_DOWN);
        c();
    }
}

void RootNode::announce()
{
    cq2::Command<NaviList> naviList(navilist_);
    naviList();

    int numItems = numberOfChildren();

    if (numItems == 0)
    {
        Narrator::Instance()->play(_N("home contains no sources"));
    }
    else if (numItems == 1)
    {
        Narrator::Instance()->setParameter("1", numItems);
        Narrator::Instance()->play(_N("home contains {1} source"));
    }
    else if (numItems > 1)
    {
        Narrator::Instance()->setParameter("2", numItems);
        Narrator::Instance()->play(_N("home contains {2} sources"));
    }
    Narrator::Instance()->playLongpause();

    announceSelection();
}

void RootNode::announceSelection()
{
    int currentChoice = 0;
    AnyNode* current = currentChild_;

    if ((firstChild() != NULL) && (current != NULL))
    {
        while (firstChild() != current)
        {
            currentChoice++;
            current = current->prev_;
        }

        Narrator::Instance()->setParameter("1", currentChoice + 1);
        Narrator::Instance()->play(_N("source no. {1}"));
        currentChild_->narrateName();

        NaviListItem item = navilist_.items[currentChoice];
        cq2::Command<NaviListItem> naviItem(item);
        naviItem();
    }
}
