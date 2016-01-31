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

#include "MP3FileNode.h"
#include "Defines.h"
#include "Utils.h"

#include <Narrator.h>
#include <NaviEngine.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr mp3FileNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.mp3filenode"));

using namespace naviengine;

MP3FileNode::MP3FileNode()
{
    LOG4CXX_TRACE(mp3FileNodeLog, "Constructor mp3filenode default");
    mp3File = "";
}

MP3FileNode::MP3FileNode(std::string file)
{
    LOG4CXX_TRACE(mp3FileNodeLog, "Constructor mp3filenode");
    mp3File = file;
}

MP3FileNode::~MP3FileNode()
{
    LOG4CXX_TRACE(mp3FileNodeLog, "Destructor mp3filenode");
}

bool MP3FileNode::up(NaviEngine& navi)
{
    return VirtualMenuNode::up(navi);
}

bool MP3FileNode::prev(NaviEngine& navi)
{
    return VirtualMenuNode::prev(navi);
}

bool MP3FileNode::next(NaviEngine& navi)
{
    return VirtualMenuNode::next(navi);
}

bool MP3FileNode::select(NaviEngine& navi)
{
    return VirtualMenuNode::select(navi);
}

bool MP3FileNode::selectByUri(naviengine::NaviEngine& navi, std::string uri)
{
    return VirtualMenuNode::selectByUri(navi, uri);
}

bool MP3FileNode::menu(NaviEngine& navi)
{
    return VirtualMenuNode::menu(navi);
}

bool MP3FileNode::onOpen(NaviEngine& navi)
{
    return VirtualMenuNode::onOpen(navi);
}

bool MP3FileNode::onNarrate()
{
    // Parent node does the narration
    return true;
}

bool MP3FileNode::narrateName()
{
    LOG4CXX_TRACE(mp3FileNodeLog, "Playing mp3 " << mp3File);

    Narrator::Instance()->playFile(mp3File);

    return true;
}
