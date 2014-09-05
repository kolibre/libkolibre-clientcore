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

#include "SleepTimerNode.h"
#include "config.h"
#include "../Defines.h"
#include "../ClientCore.h"
#include "../NaviList.h"
#include "../CommandQueue2/CommandQueue.h"
#include "../Commands/InternalCommands.h"

#include <Narrator.h>

#include <sstream>
#include <libintl.h>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr sleepTimeNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.sleeptimernode"));

using namespace naviengine;

SleepTimerNode::SleepTimerNode(ClientCore* clientcore, const std::string& name, const std::string& playBeforeOnOpen) :
        VirtualContextMenuNode(name, playBeforeOnOpen),
        clientcore_(clientcore)
{
    isOpen = false;
    currentChild = 0;

    // create virtual child nodes
    children.push_back(VirtualNode(_("sleep timer off")));
    children.push_back(VirtualNode(_("sleep timer 15 minutes")));
    children.push_back(VirtualNode(_("sleep timer 30 minutes")));
    children.push_back(VirtualNode(_("sleep timer 60 minutes")));
    states[0] = -1;
    states[1] = 15;
    states[2] = 30;
    states[3] = 60;
}

bool SleepTimerNode::up(NaviEngine& navi)
{
    isOpen = false;
    return VirtualMenuNode::up(navi);
}

bool SleepTimerNode::select(NaviEngine& navi)
{
    clientcore_->setSleepTimerTimeLeft(states[currentChild]);

    isOpen = false;
    return VirtualMenuNode::up(navi);
}

bool SleepTimerNode::selectByUri(naviengine::NaviEngine& navi, std::string uri)
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

bool SleepTimerNode::next(NaviEngine& navi)
{
    VirtualMenuNode::next(navi);
    renderChild();
    return true;
}

bool SleepTimerNode::prev(NaviEngine& navi)
{
    VirtualMenuNode::prev(navi);
    renderChild();
    return true;
}

bool SleepTimerNode::onOpen(NaviEngine&)
{
    currentChild = 0;
    isOpen = true;
    render();
    renderChild();
    return true;
}

bool SleepTimerNode::narrateInfo()
{
    const bool isSelfNarrated = true;
    if (isOpen)
    {
        Narrator::Instance()->play(info_.c_str());
        Narrator::Instance()->playShortpause();
        renderChild();
    }
    return isSelfNarrated;
}

bool SleepTimerNode::onNarrate()
{
    const bool isSelfNarrated = false;
    return isSelfNarrated;
}

void SleepTimerNode::render()
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

void SleepTimerNode::renderChild()
{
    NaviListItem item(children[currentChild].uri_, children[currentChild].name_);
    cq2::Command<NaviListItem> naviItem(item);
    naviItem();

    Narrator::Instance()->play(_N("option"));
    Narrator::Instance()->play(currentChild + 1);

    if (currentChild == 0)
    {
        Narrator::Instance()->play(_N("disable sleep timer"));
    }
    else
    {
        Narrator::Instance()->play(_N("enable sleep timer"));
        Narrator::Instance()->playDuration(0, states[currentChild], 0);
    }
}
