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

#include "GotoPageNode.h"
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
log4cxx::LoggerPtr gotoPageNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.gotopagenode"));

using namespace naviengine;

GotoPageNode::GotoPageNode(int max, DaisyNavi* daisyNavi) :
        iMax(max), daisyNavi(daisyNavi)
{
    isOpen = false;
    currentChild = 0;

    mapNarrations[NARRATE_ENTER] = _N("please enter");
    mapNarrations[NARRATE_TYPE] = _N("page");
    mapNarrations[NARRATE_TYPES] = _N("pages");
    mapNarrations[NARRATE_GOTO] = _N("jump to");
    mapNarrations[NARRATE_GOINGTO] = _N("jumping to");

    name_ = _N("jump to page");
    info_ = _N("choose page using left and right arrows, jump to selected page using play button");

    // create virtual childs
    for (int i = 0; i < iMax; i++)
    {
        children.push_back(VirtualNode(daisyNavi->getPageLabel(i)));
    }
}

/*
 * Exit goto node
 */
bool GotoPageNode::up(NaviEngine& navi)
{
    isOpen = false;
    return VirtualMenuNode::up(navi);
}

/*
 * Jump to the selected page
 */
bool GotoPageNode::select(NaviEngine& navi)
{
    Narrator::Instance()->play(mapNarrations[NARRATE_GOINGTO].c_str());
    narrateValue(currentChild, false);

    // call the jumptopagenumber for currentChild
    string pageId = daisyNavi->getPageId(currentChild);
    if (not pageId.empty())
    {
        cq2::Command<JumpCommand<std::string> > jump(pageId);
        jump();
    }
    else
    {
        LOG4CXX_ERROR(gotoPageNodeLog, "Unable to jump to page, pageId id is empty");
    }

    return navi.closeMenu();
}

bool GotoPageNode::selectByUri(naviengine::NaviEngine& navi, std::string uri)
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

bool GotoPageNode::next(NaviEngine& navi)
{
    VirtualMenuNode::next(navi);
    renderChild();
    return true;
}

bool GotoPageNode::prev(NaviEngine& navi)
{
    VirtualMenuNode::prev(navi);
    renderChild();
    return true;
}

/*
 * Reset the node to start selecting target jump position
 * based on current time in book
 */
bool GotoPageNode::onOpen(NaviEngine&)
{
    //(Re)Initialize node
    isOpen = true;

    // initialize currentChild to current selection
    currentChild = daisyNavi->getCurrentPageIdx();
    render();
    renderChild(true);
    narrateEnterType();
    return true;
}

void GotoPageNode::beforeOnOpen()
{
    Narrator::Instance()->play(_N("opening jump to page"));
}

bool GotoPageNode::onNarrate()
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

void GotoPageNode::render()
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

void GotoPageNode::renderChild(bool silent)
{
    NaviListItem item(children[currentChild].uri_, children[currentChild].name_);
    cq2::Command<NaviListItem> naviItem(item);
    naviItem();

    if (not silent)
        narrateValue(currentChild, false);
}

/*
 * Narrate current target page and total available pages
 */
void GotoPageNode::narrateValue(long aValue, bool bNarrateMax)
{
    int aPage;
    aPage = atoi(daisyNavi->getPageLabel(aValue).c_str());
    if (aPage > 0)
    {
        if (bNarrateMax)
        {
            Narrator::Instance()->play(_N("highest page number in this book"));
        }
        Narrator::Instance()->play(mapNarrations[NARRATE_TYPE].c_str());
        Narrator::Instance()->play(aPage);
    }
    else
    {
        Narrator::Instance()->play(_N("special page"));
    }
}

/*
 * Ask the user to input correct type
 */
void GotoPageNode::narrateEnterType()
{
    Narrator::Instance()->play(mapNarrations[NARRATE_ENTER].c_str());
    Narrator::Instance()->play(mapNarrations[NARRATE_TYPE].c_str());
}
