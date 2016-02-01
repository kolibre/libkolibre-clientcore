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
#include "Defines.h"
#include "config.h"
#include "CommandQueue2/CommandQueue.h"
#include "Commands/InternalCommands.h"
#include "Utils.h"
#include "Settings/Settings.h"

#include <Narrator.h>
#include <NaviEngine.h>

#include <algorithm>
#include <vector>
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
    VirtualMenuNode::next(navi);
    //renderChild();
    announceSelection();
    return true;
}

bool MP3Node::prev(NaviEngine& navi)
{
    VirtualMenuNode::prev(navi);
    //renderChild();
    announceSelection();
    return true;
}

bool MP3Node::menu(NaviEngine& navi)
{
    return navi.openMenu(navi.buildContextMenu());
}

bool MP3Node::up(NaviEngine& navi)
{
    pathUpdated_ = false;
    return VirtualMenuNode::up(navi);
}

bool MP3Node::select(naviengine::NaviEngine&)
{
    return true;
}

bool MP3Node::selectByUri(naviengine::NaviEngine&, std::string)
{
    return true;
}

bool MP3Node::onOpen(NaviEngine& navi)
{
    if (not pathUpdated_)
    {
        // Create sources defined in MediaSourceManager
        LOG4CXX_INFO(mp3NodeLog, "Searching for supported content in path '" << fsPath_ << "'");

        std::vector<std::string> uris = Utils::recursiveSearchByExtension(fsPath_, ".mp3");
        std::sort(uris.begin(), uris.end());
        for (int i = 0; i < uris.size(); i++)
        {
            // create virtual childs
            LOG4CXX_DEBUG(mp3NodeLog, "Creating mp3 track: '" <<  uris[i] << "'");

            // invent a name for it
            std::string title = "mp3";
            // a better title would be the name of the file (uris - filepath)
            ostringstream oss;
            oss << "track_";
            oss << (i+1);
            oss << "_";
            oss << title;

            children.push_back(VirtualNode(oss.str()));
            files[i] = uris[i];
        }

        currentChild = 0;
        pathUpdated_ = true;
    }

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
        Narrator::Instance()->playFile(files[currentChild]);
    }
}

void MP3Node::announce()
{
    int numItems = numberOfChildren();

    if (numItems == 0)
    {
        Narrator::Instance()->play(_N("directory contains no tracks"));
    }
    else if (numItems == 1)
    {
        Narrator::Instance()->setParameter("1", numItems);
        Narrator::Instance()->play(_N("directory contains {1} track"));
    }
    else if (numItems > 1)
    {
        Narrator::Instance()->setParameter("2", numItems);
        Narrator::Instance()->play(_N("directory contains {2} tracks"));
    }
    Narrator::Instance()->playLongpause();

    announceSelection();
}

void MP3Node::announceSelection()
{
    Narrator::Instance()->setParameter("1", currentChild + 1);
    Narrator::Instance()->play(_N("track no. {1}"));

    // wait for narrator to finnish before playing mp3
    while (Narrator::Instance()->isSpeaking()) usleep(100000);
    Narrator::Instance()->playFile(files[currentChild]);
}
