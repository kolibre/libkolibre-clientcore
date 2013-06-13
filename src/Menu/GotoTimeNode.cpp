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

#include "GotoTimeNode.h"
#include "config.h"
#include "../DaisyNavi.h"
#include "../Defines.h"
#include "../NaviList.h"
#include "../CommandQueue2/CommandQueue.h"
#include "../Commands/JumpCommand.h"

#include <Narrator.h>

#include <math.h>
#include <libintl.h>
#include <iomanip>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr gotoTimeNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.gototimenode"));

using namespace naviengine;

struct NaviListItem_GotoTimeNode: public NaviListItem
{
    NaviListItem_GotoTimeNode(const map<int, int>::iterator& it, int hours, int minutes, int seconds, int timeUnit)
    {
        std::ostringstream ossName;
        std::ostringstream ossUri;
        int iCurrentValue = 0;

        switch (timeUnit)
        {
        case GotoTimeNode::HOURS:
            iCurrentValue = hours;
            ossName << it->second;
            ossName << ":--:--";
            break;
        case GotoTimeNode::MINUTES:
            iCurrentValue = minutes;
            ossName << setfill('0') << setw(2) << hours;
            ossName << ":";
            ossName << setfill('0') << setw(2) << it->second;
            ossName << ":--";
            break;
        case GotoTimeNode::SECONDS:
            iCurrentValue = seconds;
            ossName << setfill('0');
            ossName << setw(2) << hours;
            ossName << ":";
            ossName << setw(2) << minutes << ":";
            ossName << setfill('0') << setw(2) << it->second;
            break;
        }

        ossUri << it->first;

        name_ = ossName.str();
        uri_ = ossUri.str();
    }
};

GotoTimeNode::GotoTimeNode(int max, DaisyNavi* daisyNavi) :
        iMax(max), daisyNavi(daisyNavi)
{
    isOpen = false;
    iCurrentSelectedSeconds = 0;
    iCurrentSelectedMinutes = 0;
    iCurrentSelectedHours = 0;

    mapNarrations[NARRATE_ENTER] = _N("please enter");
    mapNarrations[NARRATE_TYPE] = _N("time");
    mapNarrations[NARRATE_GOTO] = _N("jump to");
    mapNarrations[NARRATE_GOINGTO] = _N("jumping to");
    mapNarrations[NARRATE_MINUTES] = _N("minutes");
    mapNarrations[NARRATE_HOURS] = _N("hours");
    mapNarrations[NARRATE_SECONDS] = _N("seconds");

    iBookTotalTimeSeconds = iMax;
    name_ = _N("jump to time");
    info_ = _N("choose time using left and right arrows, jump to selected time using play button");
}

/*
 * Increase current selection level or exit node if there are no
 * higher levels
 */
bool GotoTimeNode::up(NaviEngine& navi)
{
    // Never go higher than iTimeUnit 2 or bigger that total time of book
    if (iTimeUnit < HOURS && pow(60, iTimeUnit + 1) <= iBookTotalTimeSeconds)
    {
        // Increase time unit one step
        iTimeUnit++;
        // Set iBase to max accepted value
        setLevelMax();

        narrateEnterType();
        render();
    }
    else
    {
        isOpen = false;
        return VirtualMenuNode::up(navi);
    }
    return true;
}
/*
 * Decrease current selection level or jump to the entered place in book
 */
bool GotoTimeNode::select(NaviEngine& navi)
{
    // If iTimeUnit is larger than 0 then we use numbergrouping by iBase.
    // This currently only happens when we enter a time
    if (iTimeUnit > SECONDS)
    {
        // Decrease time unit one step
        iTimeUnit--;
        // Set levelMax to max accepted value
        setLevelMax();

        render();
        renderChild(true), narrateEnterType();
        return false;
    }
    else
    {
        Narrator::Instance()->play(mapNarrations[NARRATE_GOINGTO].c_str());
        narrateValue(getSeconds(), false);

        unsigned int second = getSeconds();
        cq2::Command<JumpCommand<unsigned int> > jump(second);
        jump();

        return navi.closeMenu();
    }
}

bool GotoTimeNode::selectByUri(naviengine::NaviEngine& navi, std::string uri)
{
    int selection = atoi(uri.c_str());

    for (std::map<int, int>::iterator it = time.begin(); it != time.end(); ++it)
    {
        if (selection == (*it).first)
        {
            switch (iTimeUnit)
            {
            case HOURS:
                iCurrentSelectedHours = selection;
                break;
            case MINUTES:
                iCurrentSelectedMinutes = selection;
                break;
            case SECONDS:
                iCurrentSelectedSeconds = selection;
                break;
            }
            return select(navi);
        }
    }
    return false;
}

bool GotoTimeNode::next(NaviEngine&)
{
    switch (iTimeUnit)
    {
    case HOURS:
        if (getSeconds() == iCurrentTime)
        {
            iCurrentSelectedSeconds = 0;
            iCurrentSelectedMinutes = 0;
        }
        iCurrentSelectedHours++;
        iCurrentSelectedHours = (iCurrentSelectedHours + levelMax) % levelMax;
        break;
    case MINUTES:
        iCurrentSelectedMinutes++;
        iCurrentSelectedMinutes = (iCurrentSelectedMinutes + levelMax) % levelMax;
        break;
    case SECONDS:
        iCurrentSelectedSeconds++;
        iCurrentSelectedSeconds = (iCurrentSelectedSeconds + levelMax) % levelMax;
        break;
    }
    renderChild();
    return true;
}

bool GotoTimeNode::prev(NaviEngine&)
{
    switch (iTimeUnit)
    {
    case HOURS:
        if (getSeconds() == iCurrentTime)
        {
            iCurrentSelectedSeconds = 0;
            iCurrentSelectedMinutes = 0;
        }
        iCurrentSelectedHours--;
        iCurrentSelectedHours = (iCurrentSelectedHours + levelMax) % levelMax;
        break;
    case MINUTES:
        iCurrentSelectedMinutes--;
        iCurrentSelectedMinutes = (iCurrentSelectedMinutes + levelMax) % levelMax;
        break;
    case SECONDS:
        iCurrentSelectedSeconds--;
        iCurrentSelectedSeconds = (iCurrentSelectedSeconds + levelMax) % levelMax;
        break;
    }
    renderChild();
    return true;
}

/*
 * Reset the node to start selecting target jump position
 * based on current time in book
 */
bool GotoTimeNode::onOpen(NaviEngine&)
{
    int i = iBookTotalTimeSeconds;
    //divide with 60 to check how long the book is
    for (iTimeUnit = 0; iTimeUnit < HOURS && 0 < (i /= 60); iTimeUnit++)
        ;
    // Set levelMax to max accepted value
    setLevelMax();
    isOpen = true;

    iCurrentTime = daisyNavi->getCurrentTime();
    //Initialize CurSel to current selection
    iCurrentSelectedHours = iCurrentTime / 3600;
    iCurrentSelectedMinutes = (iCurrentTime / 60) % 60;
    iCurrentSelectedSeconds = iCurrentTime % 60;
    render();
    renderChild(true);
    narrateEnterType();
    return true;
}

void GotoTimeNode::beforeOnOpen()
{
    Narrator::Instance()->play(_N("opening jump to time"));
}

bool GotoTimeNode::onNarrate()
{
    bool isSelfNarrated = false;
    if (isOpen)
    {
        isSelfNarrated = true;

        Narrator::Instance()->play(info_.c_str());
        Narrator::Instance()->play(mapNarrations[NARRATE_GOTO].c_str());
        Narrator::Instance()->playLongpause();
        //Narrate current
        narrateValue(getSeconds(), false);
        //Narrate max
        narrateValue(iMax, true);
    }
    return isSelfNarrated;
}

void GotoTimeNode::render()
{
    std::string specify, unit;
    specify = _(mapNarrations[NARRATE_ENTER].c_str());
    switch (iTimeUnit)
    {
    case HOURS:
        unit = _(mapNarrations[NARRATE_HOURS].c_str());
        break;
    case MINUTES:
        unit = _(mapNarrations[NARRATE_MINUTES].c_str());
        break;
    case SECONDS:
        unit = _(mapNarrations[NARRATE_SECONDS].c_str());
        break;
    }
    NaviList list;
    list.name_ = name_ + " : " + specify + " " + unit;
    list.info_ = info_;
    time.clear();
    for (int i = 0; i < levelMax; i++)
    {
        time[i] = i;
        NaviListItem_GotoTimeNode item(time.find(i), iCurrentSelectedHours, iCurrentSelectedMinutes, iCurrentSelectedSeconds, iTimeUnit);
        list.items.push_back(item);
    }

    cq2::Command<NaviList> naviList(list);
    naviList();
}

void GotoTimeNode::renderChild(bool silent)
{
    int iCurrentValue = 0;
    switch (iTimeUnit)
    {
    case HOURS:
        iCurrentValue = iCurrentSelectedHours;
        break;
    case MINUTES:
        iCurrentValue = iCurrentSelectedMinutes;
        break;
    case SECONDS:
        iCurrentValue = iCurrentSelectedSeconds;
        break;
    }

    NaviListItem_GotoTimeNode item(time.find(iCurrentValue), iCurrentSelectedHours, iCurrentSelectedMinutes, iCurrentSelectedSeconds, iTimeUnit);
    cq2::Command<NaviListItem> naviItem(item);
    naviItem();

    if (not silent)
    {
        Narrator::Instance()->play(iCurrentValue);
        Narrator::Instance()->play(mapNarrations[NARRATE_GOTO].c_str());
        narrateValue(getSeconds(), false);
    }
}

/*
 * Sets the max value of current level so the selected
 * total wont be more than total available
 */
void GotoTimeNode::setLevelMax()
{
    switch (iTimeUnit)
    {
    case HOURS:
        levelMax = (iBookTotalTimeSeconds / 3600) + 1;
        break;
    case MINUTES:
        // Minutes available
        levelMax = (iBookTotalTimeSeconds - (iCurrentSelectedHours * 3600)) / 60 + 1;
        // Max 60 minutes
        if (levelMax > 60)
            levelMax = 60;
        break;
    case SECONDS:
        levelMax = iBookTotalTimeSeconds - (iCurrentSelectedHours * 3600) - (iCurrentSelectedMinutes * 60) + 1;
        // Max 60 seconds
        if (levelMax > 60)
            levelMax = 60;
        break;
    }

    // Restrict minutes if they are above max left
    int minutesLeft = (iBookTotalTimeSeconds - (iCurrentSelectedHours * 3600)) / 60;
    if (minutesLeft < iCurrentSelectedMinutes)
        iCurrentSelectedMinutes = minutesLeft;
    // Restrict seconds if they are above max left
    int secondsLeft = iBookTotalTimeSeconds - (iCurrentSelectedHours * 3600) - (iCurrentSelectedMinutes * 60);
    if (secondsLeft < iCurrentSelectedSeconds)
        iCurrentSelectedSeconds = secondsLeft;
}

/*
 * Narrate current target second and total available values
 */
void GotoTimeNode::narrateValue(long aValue, bool bNarrateMax)
{
    if (bNarrateMax)
    {
        Narrator::Instance()->play(_N("content total duration is"));
    }
    //Dont say 0 seconds when time is 0 hours
    if (aValue == 0)
    {
        switch (iTimeUnit)
        {
        case HOURS:
            Narrator::Instance()->playDuration(0, 0, aValue);
            break;
        case MINUTES:
            Narrator::Instance()->playDuration(0, aValue, 0);
            break;
        case SECONDS:
            Narrator::Instance()->playDuration(aValue, 0, 0);
            break;
        }
    }
    else
    {
        Narrator::Instance()->playDuration(aValue);
    }
}

/*
 * Ask the user to input correct type
 */
void GotoTimeNode::narrateEnterType()
{
    Narrator::Instance()->play(mapNarrations[NARRATE_ENTER].c_str());
    if (iTimeUnit == HOURS)
        Narrator::Instance()->play(mapNarrations[NARRATE_HOURS].c_str());
    if (iTimeUnit == MINUTES)
        Narrator::Instance()->play(mapNarrations[NARRATE_MINUTES].c_str());
    else if (iTimeUnit == SECONDS)
        Narrator::Instance()->play(mapNarrations[NARRATE_SECONDS].c_str());
}

/*
 * Calculate the target second to jump to
 * @return The selected time to jump to
 */
int GotoTimeNode::getSeconds()
{
    return iCurrentSelectedSeconds + iCurrentSelectedMinutes * 60 + iCurrentSelectedHours * 3600;
}
