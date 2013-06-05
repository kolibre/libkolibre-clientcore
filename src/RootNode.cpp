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
#include "Defines.h"
#include "config.h"
#include "CommandQueue2/CommandQueue.h"

#include <NaviEngine.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr rootNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.rootnode"));

using namespace naviengine;

RootNode::RootNode()
{
}

RootNode::~RootNode()
{
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
    currentChild_ = navi.getCurrentChoice();
    announce();

    return true;
}

bool RootNode::process(NaviEngine& navi, int command, void* data)
{
    return true;
}

bool RootNode::onNarrate()
{
    const bool isSelfNarrated = true;
    announceSelection();
    return isSelfNarrated;
}

bool RootNode::onRender()
{
    const bool isSelfRendered = true;
    return isSelfRendered;
}

void RootNode::announce()
{
    cq2::Command<NaviList> naviList(navilist_);
    naviList();

    int numItems = numberOfChildren();

    if (numItems == 0)
    {
    }
    else if (numItems == 1)
    {
    }
    else if (numItems > 1)
    {
    }

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

        NaviListItem item = navilist_.items[currentChoice];
        cq2::Command<NaviListItem> naviItem(item);
        naviItem();
    }
}
