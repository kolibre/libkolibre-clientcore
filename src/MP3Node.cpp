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

#include "MP3Node.h"
#include "MP3FileNode.h"
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
log4cxx::LoggerPtr mp3NodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.mp3node"));

using namespace naviengine;

MP3Node::MP3Node(const std::string name, const std::string path, bool openFirstChild)
{
    LOG4CXX_TRACE(mp3NodeLog, "Constructor mp3node");
    name_ = "MP3_" + name;
    fsName_ = name;
    fsPath_ = path;
    pathUpdated_ = false;
    openFirstChild_ = openFirstChild;
}

MP3Node::~MP3Node()
{
    LOG4CXX_TRACE(mp3NodeLog, "Destructor mp3node");
}

// NaviEngine functions

bool MP3Node::next(NaviEngine& navi)
{
    bool ret = MenuNode::next(navi);
    currentChild_ = navi.getCurrentChoice();
    announceSelection();
    return ret;
}

bool MP3Node::prev(NaviEngine& navi)
{
    bool ret = MenuNode::prev(navi);
    currentChild_ = navi.getCurrentChoice();
    announceSelection();
    return ret;
}

bool MP3Node::menu(NaviEngine& navi)
{
    return navi.openMenu(navi.buildContextMenu());
}

bool MP3Node::up(NaviEngine& navi)
{
    pathUpdated_ = false;
    bool ret = MenuNode::up(navi);
    return ret;
}

bool MP3Node::onOpen(NaviEngine& navi)
{
    if (not pathUpdated_)
    {
        navi.setCurrentChoice(NULL);
        clearNodes();
        navilist_.items.clear();

        // Create sources defined in MediaSourceManager
        LOG4CXX_INFO(mp3NodeLog, "Searching for supported content in path '" << fsPath_ << "'");

        std::vector<std::string> uris = Utils::recursiveSearchByExtension(fsPath_, ".mp3");
        for (int i = 0; i < uris.size(); i++)
        {
            // create book node
            LOG4CXX_DEBUG(mp3NodeLog, "Creating mp3 node: '" <<  uris[i] << "'");
            MP3FileNode* node = new MP3FileNode(uris[i]);
            std::string title = "mp3";

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
        narratorDoneConnection = Narrator::Instance()->connectAudioFinished(boost::bind(&MP3Node::onNarratorDone, this));
        Narrator::Instance()->setPushCommandFinished(true);
    }

    return true;
}

void MP3Node::beforeOnOpen()
{
    if (not pathUpdated_)
        Narrator::Instance()->play(_N("updating device"));
}

bool MP3Node::process(NaviEngine& navi, int command, void* data)
{
    return false;
}

bool MP3Node::narrateName()
{
    const bool isSelfNarrated = true;
    Narrator::Instance()->play(_N("mp3 path"));
    return isSelfNarrated;
}

bool MP3Node::narrateInfo()
{
    const bool isSelfNarrated = true;
    Narrator::Instance()->play(_N("choose option using left and right arrows, open using play button"));
    Narrator::Instance()->playLongpause();
    announceSelection();
    return isSelfNarrated;
}

bool MP3Node::onNarrate()
{
    const bool isSelfNarrated = true;
    return isSelfNarrated;
}

bool MP3Node::onRender()
{
    const bool isSelfRendered = true;
    return isSelfRendered;
}

bool MP3Node::abort()
{
    if (openFirstChild_)
    {
        openFirstChild_ = false;
        Narrator::Instance()->setPushCommandFinished(false);
        narratorDoneConnection.disconnect();
    }

    return true;
}

void MP3Node::onNarratorDone()
{
    bool autoPlay = Settings::Instance()->read<bool>("autoplay", true);
    if (autoPlay && openFirstChild_ && numberOfChildren() >= 1)
    {
        openFirstChild_ = false;
        Narrator::Instance()->setPushCommandFinished(false);
        narratorDoneConnection.disconnect();

        LOG4CXX_INFO(mp3NodeLog, "auto open first child");
        cq2::Command<INTERNAL_COMMAND> c(COMMAND_DOWN);
        c();
    }
}

void MP3Node::announce()
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
        Narrator::Instance()->play(_N("directory contains {1} track"));
    }
    else if (numItems > 1)
    {
        Narrator::Instance()->setParameter("2", numItems);
        Narrator::Instance()->play(_N("directory contains {1} track"));
    }
    Narrator::Instance()->playLongpause();

    announceSelection();
}

void MP3Node::announceSelection()
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
        Narrator::Instance()->play(_N("track no. {1}"));

        currentChild_->narrateName();

        NaviListItem item = navilist_.items[currentChoice];
        cq2::Command<NaviListItem> naviItem(item);
        naviItem();
    }
}
