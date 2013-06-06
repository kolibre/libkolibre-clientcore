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

#include "Navi.h"
#include "Defines.h"
#include "ClientCore.h"
#include "NaviListItem.h"
#include "NaviList.h"
#include "NaviListImpl.h"
#include "Menu/TempoNode.h"
#include "Menu/SleepTimerNode.h"
#include "Commands/InternalCommands.h"
#include "CommandQueue2/CommandQueue.h"

#include <Narrator.h>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr naviLog(log4cxx::Logger::getLogger("kolibre.clientcore.navi"));

Navi::Navi(ClientCore* clientcore) :
        NaviEngine(), clientcore_(clientcore)
{
    LOG4CXX_TRACE(naviLog, "Constructor");
}

Navi::~Navi()
{
    LOG4CXX_TRACE(naviLog, "Destructor");
}

bool Navi::process(int command, void* data)
{
    LOG4CXX_INFO(naviLog, "Processing command: " << command);

    switch (command)
    {
    case COMMAND_RIGHT:
    case COMMAND_LEFT:
    case COMMAND_DOWN:
    case COMMAND_UP:
    case COMMAND_OPEN_CONTEXTMENU:
    case COMMAND_PAUSE:
    case COMMAND_BACK:
    case COMMAND_HOME:
        Narrator::Instance()->stop();
        break;
    default:
        break;
    };

    switch (command)
    {
    case COMMAND_INFO:
        LOG4CXX_INFO(naviLog, "COMMAND_INFO received");
        if (not NaviEngine::process(command, data))
        {
            narrateNode();
        }
        break;

    case COMMAND_HOME:
        LOG4CXX_INFO(naviLog, "COMMAND_HOME received");
        LOG4CXX_INFO(naviLog, "Returning to top menu");
        top();
        break;

    case COMMAND_RIGHT:
        LOG4CXX_INFO(naviLog, "COMMAND_RIGHT received");
        LOG4CXX_INFO(naviLog, "Going to next item");
        next();
        break;

    case COMMAND_LEFT:
        LOG4CXX_INFO(naviLog, "COMMAND_LEFT received");
        LOG4CXX_INFO(naviLog, "Going to prev item");
        prev();
        break;

    case COMMAND_DOWN:
        LOG4CXX_INFO(naviLog, "COMMAND_DOWN received");
        LOG4CXX_INFO(naviLog, "Selecting item");
        select();
        break;

    case COMMAND_UP:
        LOG4CXX_INFO(naviLog, "COMMAND_UP received");
        LOG4CXX_INFO(naviLog, "Going parent item");
        up();
        break;

    case COMMAND_OPEN_CONTEXTMENU:
        LOG4CXX_INFO(naviLog, "COMMAND_OPEN_CONTEXTMENU received");
        LOG4CXX_INFO(naviLog, "Opening context menu");
        if (!openContextMenu())
            next();
        break;

    case COMMAND_PAUSE:
        LOG4CXX_INFO(naviLog, "COMMAND_PAUSE received");
        if (not NaviEngine::process(command, data))
        {
            select();
        }
        break;

    case COMMAND_BACK:
        LOG4CXX_INFO(naviLog, "COMMAND_BACK received");
        LOG4CXX_INFO(naviLog, "Going back");
        if (not NaviEngine::process(command, data))
        {
            up();
        }
        break;

    default:
        //This is a hack to get the COMMAND_NEXT signals sent from
        //player thread to this thread which eventually processes them
        //in DaisyNavi. Maybe DaisyNavi should be made thread safe so
        //it could handle them directly, but that might have an impact
        //on the player. Needs testing.
        return NaviEngine::process(command, data);
        break;
    }

    return true;
}

void Navi::narrateChange(const MenuState& before, const MenuState& after)
{
    /*
     * 1. Narrate the opened node and its number of children. ( if onNarrate == false )
     * 2. Narrate the current choice
     * 3. If a node's onNarrate returns true then skip narrating it.
     */

    if (before.state.currentNode == after.state.currentNode)
    {
        if (before.state.currentChoice != after.state.currentChoice)
        {
            if (after.state.currentChoice != NULL)
            {
                if (renderNode(after.state.currentNode))
                {
                    naviengine::NaviListItemImpl navilistitem(after.state.currentChoice);
                    cq2::Command<NaviListItem> naviItem(navilistitem);
                    naviItem();
                }
            }

            narrateNode(after.state.currentChoice);
        }
        return;
    }
    else
    { // Narrate both node and choice change
        if (NULL != after.state.currentNode and renderNode(after.state.currentNode))
        {
            naviengine::NaviListImpl navilist(after.state.currentNode);
            cq2::Command<NaviList> naviList(navilist);
            naviList();

            if (NULL != after.state.currentChoice)
            {
                naviengine::NaviListItemImpl navilistitem(after.state.currentChoice);
                cq2::Command<NaviListItem> naviItem(navilistitem);
                naviItem();
            }
        }

        if (not after.state.currentNode->onNarrate())
        {
            string name = after.state.currentNode->name_;
            if (name != "")
            {
                narrate(name.c_str());
                int numAlternatives = numberOfChildren(after.state.currentNode);
                if (numAlternatives > 0)
                {
                    if (numAlternatives == 1)
                    {
                        narrateSetParam("1", numAlternatives);
                        narrate(_N("contains {1} option"));
                    }
                    else
                    {
                        narrateSetParam("2", numAlternatives);
                        narrate(_N("contains {2} options"));
                    }

                    narrateShortPause();
                    narrate(_N("option"));

                    narrateNode(after.state.currentChoice);
                }
                else
                {
                    narrate(_N("contains no options"));
                }
            }
        }
    }
}

void Navi::narrate(const std::string text)
{
    Narrator::Instance()->play(text.c_str());
}

void Navi::narrate(const int value)
{
    Narrator::Instance()->play(value);
}

void Navi::narrateStop()
{
    Narrator::Instance()->stop();
}

void Navi::narrateShortPause()
{
    Narrator::Instance()->playShortpause();
}

void Navi::narrateLongPause()
{
    Narrator::Instance()->playLongpause();
}

void Navi::narrateSetParam(std::string param, int number)
{
    Narrator::Instance()->setParameter(param, number);
}

naviengine::MenuNode* Navi::buildContextMenu()
{
    using namespace naviengine;
    MenuNode* contextMenu = new MenuNode();
    contextMenu->name_ = _N("menu");
    contextMenu->play_before_onOpen_ = _N("opening main menu");
    contextMenu->info_ = _N("choose option using left and right arrows, open using play button");

    // We have so few settings that we can add them directly under context
    {
        VirtualMenuNode* subMenuNode = 0;

        //TEMPO
        subMenuNode = new TempoNode;
        subMenuNode->name_ = _N("playback speed");
        subMenuNode->play_before_onOpen_ = _N("opening playback speed menu");
        subMenuNode->info_ = _N("choose option using left and right arrows, open using play button");
        contextMenu->addNode(subMenuNode);

        //SLEEP
        subMenuNode = new SleepTimerNode(clientcore_);
        subMenuNode->name_ = _N("sleep timer");
        subMenuNode->play_before_onOpen_ = _N("opening sleep timer");
        subMenuNode->info_ = _N("choose option using left and right arrows, open using play button");
        contextMenu->addNode(subMenuNode);
    }

    return contextMenu;
}
