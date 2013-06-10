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

#include <NaviEngine.h>

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
}

DaisyBookNode::DaisyBookNode(std::string uri)
{
    LOG4CXX_TRACE(daisyBookNodeLog, "Constructor");
    pDaisyNavi = new DaisyNavi;
    daisyNaviActive = false;
    daisyUri_ = uri;
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
