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

#include "GotoPercentNode.h"
#include "config.h"
#include "../DaisyNavi.h"
#include "../Defines.h"
#include "../NaviList.h"
#include "../CommandQueue2/CommandQueue.h"
#include "../Commands/JumpCommand.h"

#include <Narrator.h>

#include <math.h>
#include <libintl.h>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr gotoPercentNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.gotopercentnode"));

using namespace naviengine;

int calculateSeconds(int total, int percent)
{
    return (total * percent) / 100;
}

GotoPercentNode::GotoPercentNode(int max, DaisyNavi* daisyNavi) :
        iMax(max), daisyNavi(daisyNavi)
{
    isOpen = false;
    currentChild = 0;

    mapNarrations[NARRATE_ENTER] = _N("please enter");
    mapNarrations[NARRATE_TYPE] = _N("percent");
    mapNarrations[NARRATE_TYPES] = _N("percent");
    mapNarrations[NARRATE_GOTO] = _N("jump to");
    mapNarrations[NARRATE_GOINGTO] = _N("jumping to");

    name_ = _N("jump to percent");
    info_ = _N("choose percent using left and right arrows, jump to selected percent using play button");

    // create virtual childs
    for (int i = 0; i <= 100; i++)
    {
        ostringstream oss;
        oss << i << "%";
        children.push_back(VirtualNode(oss.str()));
    }
}

/*
 * Exit goto node
 */
bool GotoPercentNode::up(NaviEngine& navi)
{
    isOpen = false;
    return VirtualMenuNode::up(navi);
}

/*
 * Jump to the selected percent
 */
bool GotoPercentNode::select(NaviEngine& navi)
{
    Narrator::Instance()->play(mapNarrations[NARRATE_GOINGTO].c_str());
    narrateValue(currentChild, false);

    unsigned int second = calculateSeconds(iMax, currentChild);
    cq2::Command<JumpCommand<unsigned int> > jump(second);
    jump();

    return navi.closeMenu();
}

bool GotoPercentNode::selectByUri(NaviEngine& navi, std::string uri)
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

bool GotoPercentNode::next(NaviEngine& navi)
{
    VirtualMenuNode::next(navi);
    renderChild();
    return true;
}

bool GotoPercentNode::prev(NaviEngine& navi)
{
    VirtualMenuNode::prev(navi);
    renderChild();
    return true;
}

/*
 * Reset the node to start selecting target jump position
 * based on current time in book
 */
bool GotoPercentNode::onOpen(NaviEngine&)
{
    //(Re)Initialize node
    isOpen = true;

    // Initialize currentChild to current selection
    currentChild = daisyNavi->getCurrentPercent();
    render();
    renderChild(true);
    narrateEnterType();
    return true;
}

void GotoPercentNode::beforeOnOpen()
{
    Narrator::Instance()->play(_N("opening jump to percent"));
}

bool GotoPercentNode::onNarrate()
{
    bool isSelfNarrated = false;
    if (isOpen)
    {
        isSelfNarrated = true;

        Narrator::Instance()->play(info_.c_str());
        Narrator::Instance()->play(mapNarrations[NARRATE_GOTO].c_str());
        Narrator::Instance()->playLongpause();
        //Narrate current
        narrateValue(currentChild, false);
        //Narrate max
        narrateValue(iMax, true);
    }
    return isSelfNarrated;
}

void GotoPercentNode::render()
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
}

void GotoPercentNode::renderChild(bool silent)
{
    NaviListItem item(children[currentChild].uri_, children[currentChild].name_);
    cq2::Command<NaviListItem> naviItem(item);
    naviItem();

    if (not silent)
        narrateValue(currentChild, false);
}

/*
 * Narrate current target percent and total available percent
 */
void GotoPercentNode::narrateValue(long aValue, bool bNarrateMax)
{
    if (bNarrateMax)
    {
        // don't narrate anything
    }
    else
    {
        Narrator::Instance()->play(aValue);
        Narrator::Instance()->play(mapNarrations[NARRATE_TYPES].c_str());
    }
}

/*
 * Ask the user to input correct type
 */
void GotoPercentNode::narrateEnterType()
{
    Narrator::Instance()->play(mapNarrations[NARRATE_ENTER].c_str());
    Narrator::Instance()->play(mapNarrations[NARRATE_TYPE].c_str());
}
