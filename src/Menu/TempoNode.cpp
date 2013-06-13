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

#include "TempoNode.h"
#include "config.h"
#include "../Defines.h"
#include "../NaviList.h"
#include "../Settings/Settings.h"
#include "../CommandQueue2/CommandQueue.h"
#include "../Commands/InternalCommands.h"

#include <Narrator.h>
#include <Player.h>

#include <libintl.h>
#include <algorithm>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr tempoNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.temponode"));

using namespace naviengine;

int calculateSelection(double curr)
{
    return (int) (curr * 10.0 - 10);
}

double calculateTempo(int selection)
{
    return (double) (selection / 10.0) + 1;
}

TempoNode::TempoNode(const std::string& name, const std::string playBeforeOnOpen) :
        VirtualContextMenuNode(name, playBeforeOnOpen)
{
    isOpen = false;
    currentChild = 0;

    // calculate minimum/maximum common tempo
    double minTempo = std::min(NARRATOR_MIN_TEMPO, PLAYER_MIN_TEMPO);
    double maxTempo = std::max(NARRATOR_MAX_TEMPO, PLAYER_MAX_TEMPO);

    // create virtual child nodes
    int option = 0;
    for (int speed = calculateSelection(minTempo); speed <= calculateSelection(maxTempo); ++speed)
    {
        ostringstream oss;
        oss << speed;
        children.push_back(VirtualNode(oss.str()));
        selections[option++] = speed;
    }
}

bool TempoNode::up(NaviEngine& navi)
{
    // undo tempo changes
    double tempo = calculateTempo(currentTempo);
    LOG4CXX_INFO(tempoNodeLog, "restore tempo to " << currentTempo << " (" << tempo << ")");
    Narrator::Instance()->setTempo(tempo);

    isOpen = false;
    return VirtualMenuNode::up(navi);
}

bool TempoNode::select(NaviEngine& navi)
{
    // store tempo to settings
    double tempo = calculateTempo(selections[currentChild]);
    LOG4CXX_INFO(tempoNodeLog, "changing tempo to " << selections[currentChild] << " (" << tempo << ")");
    Narrator::Instance()->setTempo(tempo);
    Player::Instance()->setTempo(tempo);
    Settings::Instance()->write<double>("playbackspeed", tempo);

    Narrator::Instance()->play(_N("current speed"));

    // Announce plus if speed is larger than 0
    if (selections[currentChild] > 0)
        Narrator::Instance()->play(_N("plus"));
    if (selections[currentChild] != 0)
        Narrator::Instance()->play(selections[currentChild]);
    else
        Narrator::Instance()->play(_N("normal speed"));

    isOpen = false;
    return VirtualMenuNode::up(navi);
}

bool TempoNode::selectByUri(naviengine::NaviEngine& navi, std::string uri)
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

bool TempoNode::next(NaviEngine& navi)
{
    VirtualMenuNode::next(navi);
    // change tempo, current tempo restored on exit
    double tempo = calculateTempo(selections[currentChild]);
    LOG4CXX_INFO(tempoNodeLog, "changing tempo to " << selections[currentChild] << " (" << tempo << ")");
    Narrator::Instance()->setTempo(tempo);
    renderChild();
    return true;
}

bool TempoNode::prev(NaviEngine& navi)
{
    VirtualMenuNode::prev(navi);
    // change tempo, current tempo restored on exit
    double tempo = calculateTempo(selections[currentChild]);
    LOG4CXX_INFO(tempoNodeLog, "changing tempo to " << selections[currentChild] << " (" << tempo << ")");
    Narrator::Instance()->setTempo(tempo);
    renderChild();
    return true;
}

bool TempoNode::onOpen(NaviEngine&)
{
    isOpen = true;
    double curr = Settings::Instance()->read<double>("playbackspeed", 1.0);
    currentTempo = calculateSelection(curr);
    for (std::map<int, int>::iterator it = selections.begin(); it != selections.end(); ++it)
    {
        if (currentTempo == (*it).second)
        {
            currentChild = (*it).first;
            break;
        }
    }
    render();
    renderChild();
    return true;
}

bool TempoNode::onNarrate()
{
    if (not isOpen)
        return false;

    Narrator::Instance()->play(info_.c_str());
    Narrator::Instance()->playShortpause();
    renderChild();
    return true;
}

void TempoNode::render()
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

void TempoNode::renderChild()
{
    NaviListItem item(children[currentChild].uri_, children[currentChild].name_);
    cq2::Command<NaviListItem> naviItem(item);
    naviItem();

    Narrator::Instance()->play(_N("option"));
    Narrator::Instance()->play(currentChild + 1);

    if (selections[currentChild] == 0)
    {
        Narrator::Instance()->play(_N("normal speed"));
    }
    else
    {
        Narrator::Instance()->play(_N("set speed to"));

        // Announce plus if speed is larger than 0
        if (selections[currentChild] > 0)
        {
            Narrator::Instance()->play(_N("plus"));
        }
        Narrator::Instance()->play(selections[currentChild]);
    }
}
