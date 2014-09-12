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

#include "DaisyBookNode.h"
#include "DaisyNavi.h"
#include "Defines.h"

#include <Narrator.h>
#include <NaviEngine.h>
#include <DaisyHandler.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr daisyBookNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.daisybooknode"));

using namespace naviengine;

DaisyBookNode::DaisyBookNode()
{
    LOG4CXX_TRACE(daisyBookNodeLog, "Constructor");
    pDaisyNavi = new DaisyNavi;
    daisyNaviActive = false;
    daisyUri_ = "";
    title = "";
    titleSrc = "";
}

DaisyBookNode::DaisyBookNode(std::string uri)
{
    LOG4CXX_TRACE(daisyBookNodeLog, "Constructor");
    pDaisyNavi = new DaisyNavi;
    daisyNaviActive = false;
    daisyUri_ = uri;
    title = "";
    titleSrc = "";
    initialize();
}

DaisyBookNode::~DaisyBookNode()
{
    LOG4CXX_TRACE(daisyBookNodeLog, "Destructor");
    delete pDaisyNavi;
}

bool DaisyBookNode::up(NaviEngine& navi)
{
    if (not pDaisyNavi->up(navi))
    {
        // DaisyNavi has moved beyond TOPLEVEL
        daisyNaviActive = false;
        if (pDaisyNavi->isOpen())
        {
            pDaisyNavi->closeBook();
        }

        // DaisyNavi fiddled with the state so we reset it.
        navi.setCurrentNode(this);

        // Move up one more level since the book node doesn't do
        // anything other than setup DaisyNavi.
        return VirtualMenuNode::up(navi);
    }

    return true;
}

bool DaisyBookNode::prev(NaviEngine& navi)
{
    return pDaisyNavi->prev(navi);
}

bool DaisyBookNode::next(NaviEngine& navi)
{
    return pDaisyNavi->next(navi);
}

bool DaisyBookNode::select(NaviEngine& navi)
{
    return pDaisyNavi->select(navi);
}

bool DaisyBookNode::selectByUri(naviengine::NaviEngine& navi, std::string uri)
{
    return pDaisyNavi->selectByUri(navi, uri);
}

bool DaisyBookNode::menu(NaviEngine& navi)
{
    return pDaisyNavi->menu(navi);
}

bool DaisyBookNode::onOpen(NaviEngine& navi)
{
    if (daisyNaviActive)
    {
        return pDaisyNavi->onOpen(navi);
    }

    if (not daisyUri_.empty() && pDaisyNavi->open(daisyUri_) && pDaisyNavi->onOpen(navi))
    {
        LOG4CXX_INFO(daisyBookNodeLog, "Opening Daisy book with uri '" << daisyUri_ << "'");
        daisyNaviActive = true;
        return daisyNaviActive;
    }

    daisyNaviActive = false;
    return daisyNaviActive;
}

void DaisyBookNode::beforeOnOpen()
{
    if (not daisyNaviActive)
        Narrator::Instance()->play(_N("opening publication"));
}

bool DaisyBookNode::process(NaviEngine& navi, int command, void* data)
{
    bool result = pDaisyNavi->process(navi, command, data);

    // If the book is no longer opened then swap node to parent
    if (daisyNaviActive && !pDaisyNavi->isOpen())
    {
        LOG4CXX_INFO(daisyBookNodeLog, "Closing at the end of book");
        daisyNaviActive = false;
        return VirtualMenuNode::up(navi);
    }

    return result;
}

bool DaisyBookNode::onNarrate()
{
    // Parent node does the narration
    return true;
}

bool DaisyBookNode::narrateName()
{
    if (not titleSrc.empty())
    {
        usleep(100000); while (Narrator::Instance()->isSpeaking()) usleep(100000);
        std::string extension = getFileExtension(titleSrc);
        if (extension == "ogg")
        {
            Narrator::Instance()->playFile(titleSrc);
            return true;
        }
        else
        {
            LOG4CXX_WARN(daisyBookNodeLog, "file extension '" << extension << "' not supported");
        }
    }
    if(not title.empty())
    {
        LOG4CXX_DEBUG(daisyBookNodeLog, "spell title: " << title);
        Narrator::Instance()->playLongpause();
        Narrator::Instance()->spell(title.c_str());
        return true;
    }

    return false;
}

void DaisyBookNode::initialize()
{
    LOG4CXX_INFO(daisyBookNodeLog, "Initializing book node");
    amis::DaisyHandler *dh = amis::DaisyHandler::Instance();
    if (not dh->openBook(daisyUri_))
    {
        LOG4CXX_WARN(daisyBookNodeLog, "Failed to open book " << daisyUri_);
        return;
    }
    while (dh->getState() == amis::DaisyHandler::HANDLER_OPENING)
    {
        usleep(100000);
    }
    bool withBookmarks = false;
    if (not dh->setupBook(withBookmarks))
    {
        LOG4CXX_WARN(daisyBookNodeLog, "Failed to setup book " << daisyUri_);
        dh->closeBook();
        return;
    }
    title = dh->getBookInfo()->mTitle;
    titleSrc = dh->getTitleSrc();
    dh->closeBook();
}

std::string DaisyBookNode::getBookTitle()
{
    return title;
}

std::string DaisyBookNode::getBookTitleSrc()
{
    return titleSrc;
}

std::string DaisyBookNode::getFileExtension(std::string& filename)
{
    int start = filename.length() - 3;
    if (start < 0) return "";
    return filename.substr(start, filename.length());
}
