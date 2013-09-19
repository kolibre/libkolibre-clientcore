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

#include "AutoPlayNode.h"
#include "config.h"
#include "../Defines.h"
#include "../NaviList.h"
#include "../Settings/Settings.h"
#include "../CommandQueue2/CommandQueue.h"
#include "../Commands/InternalCommands.h"

#include <Narrator.h>

#include <libintl.h>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr autoPlayNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.autoplaynode"));

using namespace naviengine;

AutoPlayNode::AutoPlayNode()
{
    isOpen = false;
    currentChild = 0;

    // create virtual child nodes
    children.push_back(VirtualNode(_("auto play off")));
    children.push_back(VirtualNode(_("auto play on")));

    selections[0] = false;
    selections[1] = true;
}

bool AutoPlayNode::up(NaviEngine& navi)
{
    isOpen = false;
    return VirtualMenuNode::up(navi);
}

bool AutoPlayNode::select(NaviEngine& navi)
{
    Settings::Instance()->write<bool>("autoplay", selections[currentChild]);

    if (currentChild == 0)
        Narrator::Instance()->play(_N("auto play disabled"));
    else
        Narrator::Instance()->play(_N("auto play enabled"));

    isOpen = false;
    return VirtualMenuNode::up(navi);
}

bool AutoPlayNode::selectByUri(naviengine::NaviEngine& navi, std::string uri)
{
    for (int i = 0; i < children.size(); i++)
    {
        if (uri == children[i].uri_)
        {
            currentChild = i;
            return select(navi);
        }
    }
    return false;
}

bool AutoPlayNode::next(NaviEngine& navi)
{
    VirtualMenuNode::next(navi);
    renderChild();
    return true;
}

bool AutoPlayNode::prev(NaviEngine& navi)
{
    VirtualMenuNode::prev(navi);
    renderChild();
    return true;
}

bool AutoPlayNode::onOpen(NaviEngine&)
{
    currentChild = 0;
    isOpen = true;
    render();
    renderChild();
    return true;
}

bool AutoPlayNode::onNarrate()
{
    if (not isOpen)
        return false;

    Narrator::Instance()->play(info_.c_str());
    Narrator::Instance()->playShortpause();
    renderChild();
    return true;
}

void AutoPlayNode::render()
{
    NaviList list;
    list.name_ = name_;
    list.info_ = info_;

    for (int i = 0; i < children.size(); i++)
    {
        NaviListItem item(children[i].uri_, children[i].name_);
        list.items.push_back(item);
    }

    cq2::Command<NaviList> naviList(list);
    naviList();

    Narrator::Instance()->play(name_.c_str());
    Narrator::Instance()->setParameter("2", list.items.size());
    Narrator::Instance()->play(_N("contains {2} options"));

    Narrator::Instance()->playShortpause();
}

void AutoPlayNode::renderChild()
{
    NaviListItem item(children[currentChild].uri_, children[currentChild].name_);
    cq2::Command<NaviListItem> naviItem(item);
    naviItem();

    Narrator::Instance()->play(_N("option"));
    Narrator::Instance()->play(currentChild + 1);

    if (currentChild == 0)
        Narrator::Instance()->play(_N("disable auto play"));
    else
        Narrator::Instance()->play(_N("enable auto play"));
}
