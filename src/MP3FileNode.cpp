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
//#include "DaisyNavi.h"
#include "Defines.h"
#include "Utils.h"

#include <Narrator.h>
#include <NaviEngine.h>
#include <DaisyHandler.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr mp3FileNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.mp3filenode"));

using namespace naviengine;

MP3FileNode::MP3FileNode()
{
    LOG4CXX_TRACE(mp3FileNodeLog, "Constructor mp3filenode default");
    //pDaisyNavi = new DaisyNavi;
    daisyNaviActive = false;
    daisyUri_ = "";
    title = "";
    titleSrc = "";
}

MP3FileNode::MP3FileNode(std::string uri)
{
    LOG4CXX_TRACE(mp3FileNodeLog, "Constructor mp3filenode");
    //pDaisyNavi = new DaisyNavi;
    daisyNaviActive = false;
    daisyUri_ = uri;
    title = "";
    titleSrc = "";
    initialize();
}

MP3FileNode::~MP3FileNode()
{
    LOG4CXX_TRACE(mp3FileNodeLog, "Destructor mp3filenode");
    //delete pDaisyNavi;
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
    //Narrator::Instance()->playFile(daisyUri_);
    return VirtualMenuNode::onOpen(navi);
}

void MP3FileNode::beforeOnOpen()
{
    Narrator::Instance()->play(_N("opening publication"));
}

bool MP3FileNode::process(NaviEngine& navi, int command, void* data)
{
    return VirtualMenuNode::process(navi, command, data);
}

bool MP3FileNode::onNarrate()
{
    // Parent node does the narration
    return true;
}

bool MP3FileNode::narrateName()
{
//    if (not titleSrc.empty())
//    {
//        std::string extension = Utils::fileExtension(titleSrc);
//        if (extension == "ogg")
//        {
//            Narrator::Instance()->playFile(titleSrc);
//            return true;
//        }
//        else if (extension == "mp3")
//        {
//            Narrator::Instance()->playFile(titleSrc);
//            return true;
//        }
//        else
//        {
//            LOG4CXX_WARN(mp3FileNodeLog, "file extension '" << extension << "' not supported");
//        }
//    }
//    if(not title.empty())
//    {
//        LOG4CXX_DEBUG(mp3FileNodeLog, "spell title: " << title);
//        Narrator::Instance()->playLongpause();
//        Narrator::Instance()->spell(title.c_str());
//        return true;
//    }
    LOG4CXX_TRACE(mp3FileNodeLog, "Playing mp3 " << daisyUri_);

    Narrator::Instance()->playFile(daisyUri_);

    return true;
}

void MP3FileNode::initialize()
{

}

std::string MP3FileNode::getBookTitle()
{
    return title;
}

std::string MP3FileNode::getBookTitleSrc()
{
    return titleSrc;
}
