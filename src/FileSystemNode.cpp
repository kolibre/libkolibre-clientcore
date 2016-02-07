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

#include "FileSystemNode.h"
#include "DaisyBookNode.h"
#include "Defines.h"
#include "config.h"
#include "CommandQueue2/CommandQueue.h"
#include "Commands/InternalCommands.h"
#include "Utils.h"
#include "Settings/Settings.h"

#include <Narrator.h>
#include <NaviEngine.h>

#include <sstream>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr fsNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.filesystemnode"));

using namespace naviengine;

FileSystemNode::FileSystemNode(const std::string name, const std::string path, bool openFirstChild)
{
    LOG4CXX_TRACE(fsNodeLog, "Constructor");
    name_ = "FileSystem_" + name;
    fsName_ = name;
    fsPath_ = path;
    pathUpdated_ = false;
    openFirstChild_ = openFirstChild;
}

FileSystemNode::~FileSystemNode()
{
    LOG4CXX_TRACE(fsNodeLog, "Destructor");
}

// NaviEngine functions

bool FileSystemNode::next(NaviEngine& navi)
{
    bool ret = MenuNode::next(navi);
    currentChild_ = navi.getCurrentChoice();
    announceSelection();
    return ret;
}

bool FileSystemNode::prev(NaviEngine& navi)
{
    bool ret = MenuNode::prev(navi);
    currentChild_ = navi.getCurrentChoice();
    announceSelection();
    return ret;
}

bool FileSystemNode::menu(NaviEngine& navi)
{
    return navi.openMenu(navi.buildContextMenu());
}

bool FileSystemNode::up(NaviEngine& navi)
{
    pathUpdated_ = false;
    bool ret = MenuNode::up(navi);
    return ret;
}

bool FileSystemNode::onOpen(NaviEngine& navi)
{
    if (not pathUpdated_)
    {
        navi.setCurrentChoice(NULL);
        clearNodes();
        navilist_.items.clear();

        // Create sources defined in MediaSourceManager
        LOG4CXX_INFO(fsNodeLog, "Searching for supported content in path '" << fsPath_ << "'");

        // Recursively search for Daisy2.02 publications
        std::vector<std::string> uris = Utils::recursiveSearchByFilename(fsPath_, "ncc.html");
        LOG4CXX_INFO(fsNodeLog, "Found " << uris.size() << " matches by file name");
        for (int i = 0; i < uris.size(); i++)
        {
            // create book node
            LOG4CXX_DEBUG(fsNodeLog, "Creating book node: '" <<  uris[i] << "'");
            DaisyBookNode* node = new DaisyBookNode(uris[i]);
            std::string title = node->getBookTitle();

            // invent a name for it
            ostringstream oss;
            oss << (i+1);
            node->name_ = "title_" + oss.str() + "_" + title;

            // add node
            addNode(node);

            // create a NaviListItem and store it in list for the NaviList signal
            NaviListItem item(node->uri_, node->name_);
            navilist_.items.push_back(item);

        }

        // Recursively search for Daisy3 publications
        uris = Utils::recursiveSearchByExtension(fsPath_, ".opf");
        LOG4CXX_INFO(fsNodeLog, "Found " << uris.size() << " matches by file extension");
        for (int i = 0; i < uris.size(); i++)
        {
            // create book node
            LOG4CXX_DEBUG(fsNodeLog, "Creating book node: '" <<  uris[i] << "'");
            DaisyBookNode* node = new DaisyBookNode(uris[i]);
            std::string title = node->getBookTitle();

            // invent a name for it
            ostringstream oss;
            oss << (i+1);
            node->name_ = "title_" + oss.str() + "_" + title;

            // add node
            addNode(node);

            // create a NaviListItem and store it in list for the NaviList signal
            NaviListItem item(node->uri_, node->name_);
            navilist_.items.push_back(item);

        }

        if (navi.getCurrentChoice() == NULL && numberOfChildren() > 0)
        {
            navi.setCurrentChoice(firstChild());
            currentChild_ = firstChild();
        }

        pathUpdated_ = true;
    }

    currentChild_ = navi.getCurrentChoice();
    announce();

    bool autoPlay = Settings::Instance()->read<bool>("autoplay", true);
    if (autoPlay && openFirstChild_)
    {
        narratorDoneConnection = Narrator::Instance()->connectAudioFinished(boost::bind(&FileSystemNode::onNarratorDone, this));
        Narrator::Instance()->setPushCommandFinished(true);
    }

    return true;
}

void FileSystemNode::beforeOnOpen()
{
    if (not pathUpdated_)
        Narrator::Instance()->play(_N("updating device"));
}

bool FileSystemNode::process(NaviEngine& navi, int command, void* data)
{
    return false;
}

bool FileSystemNode::narrateName()
{
    const bool isSelfNarrated = true;
    std::string lcName = Utils::toLower(fsName_);
    if (Utils::contains(lcName, "usb"))
        Narrator::Instance()->play(_N("usb device"));
    else if (Utils::contains(lcName, "sd"))
        Narrator::Instance()->play(_N("sd device"));
    else if (Utils::contains(lcName, "cdrom"))
        Narrator::Instance()->play(_N("cdrom device"));
    else if (Utils::contains(lcName ,"external"))
        Narrator::Instance()->play(_N("external device"));
    else
        Narrator::Instance()->play(_N("local device"));
    return isSelfNarrated;
}

bool FileSystemNode::narrateInfo()
{
    const bool isSelfNarrated = true;
    Narrator::Instance()->play(_N("choose option using left and right arrows, open using play button"));
    Narrator::Instance()->playLongpause();
    announceSelection();
    return isSelfNarrated;
}

bool FileSystemNode::onNarrate()
{
    const bool isSelfNarrated = true;
    return isSelfNarrated;
}

bool FileSystemNode::onRender()
{
    const bool isSelfRendered = true;
    return isSelfRendered;
}

bool FileSystemNode::abort()
{
    if (openFirstChild_)
    {
        openFirstChild_ = false;
        Narrator::Instance()->setPushCommandFinished(false);
        narratorDoneConnection.disconnect();
    }

    return true;
}

void FileSystemNode::onNarratorDone()
{
    bool autoPlay = Settings::Instance()->read<bool>("autoplay", true);
    if (autoPlay && openFirstChild_ && numberOfChildren() >= 1)
    {
        openFirstChild_ = false;
        Narrator::Instance()->setPushCommandFinished(false);
        narratorDoneConnection.disconnect();

        LOG4CXX_INFO(fsNodeLog, "auto open first child");
        cq2::Command<INTERNAL_COMMAND> c(COMMAND_DOWN);
        c();
    }
}

void FileSystemNode::announce()
{
    cq2::Command<NaviList> naviList(navilist_);
    naviList();

    int numItems = numberOfChildren();

    if (numItems == 0)
    {
        Narrator::Instance()->play(_N("device contains no publications"));
    }
    else if (numItems == 1)
    {
        Narrator::Instance()->setParameter("1", numItems);
        Narrator::Instance()->play(_N("device contains {1} publication"));
    }
    else if (numItems > 1)
    {
        Narrator::Instance()->setParameter("2", numItems);
        Narrator::Instance()->play(_N("device contains {2} publications"));
    }
    Narrator::Instance()->playLongpause();

    announceSelection();
}

void FileSystemNode::announceSelection()
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
        Narrator::Instance()->play(_N("publication no. {1}"));

        currentChild_->narrateName();

        NaviListItem item = navilist_.items[currentChoice];
        cq2::Command<NaviListItem> naviItem(item);
        naviItem();
    }
}
