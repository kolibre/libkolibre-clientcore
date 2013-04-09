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

#ifndef _DAISYNAVI_H
#define _DAISYNAVI_H

#include <DaisyHandler.h>
#include <Player.h>
#include <Narrator.h>
#include <Nodes/VirtualMenuNode.h>

#include <string>
#include <pthread.h>
#include <boost/signals2.hpp>

// forward declare data types
class BookInfoNode;
class Handle_JumpToSecond;

/**
 * DaisyNavi implements the VirtualMenuNode, making a book function like a menu.
 */
class DaisyNavi: public naviengine::VirtualMenuNode
{
public:
    DaisyNavi();
    ~DaisyNavi();

    // naviengine node navigation methods

    /**
     * select changes navi level
     */
    bool select(naviengine::NaviEngine&);

    /**
     * selectByUri changes position
     */
    bool selectByUri(naviengine::NaviEngine&, std::string);

    /**
     * up changes navi level
     */
    bool up(naviengine::NaviEngine&);

    /**
     * next heading or phrase
     */
    bool next(naviengine::NaviEngine&);

    /**
     * prev heading or phrase
     */
    bool prev(naviengine::NaviEngine&);

    /**
     * menu opens menu while a book is open
     */
    bool menu(naviengine::NaviEngine&);

    /**
     * opened book is loaded
     */
    bool onOpen(naviengine::NaviEngine&);

    // Interface
    bool open(const std::string &uri);
    bool closeBook();
    bool isOpen();
    void sayLevel(amis::DaisyHandler::NaviLevel level, bool verbose = false);

    bool process(naviengine::NaviEngine&, int command, void* data = 0);

    bool playAudio(std::string, long long, long long);

    enum BookmarkState
    {
        BOOKMARK_DEFAULT,
        BOOKMARK_ADD,
        BOOKMARK_INFO,
        BOOKMARK_DELETE,
        BOOKMARK_DELETEALL,
        BOOKMARK_TIMEOUT
    } bookmarkState;

    std::string getPageId(int pageNumber);
    std::string getPageLabel(int pageNumber);
    int getCurrentTime();
    int getCurrentPageIdx();
    int getCurrentPercent();
    bool bPlaybackIsPaused;

private:
    // signals and slots
    bool playerMessageSlot(Player::playerMessage);
    bool playerTimeSlot(Player::timeData td);
    boost::signals2::connection playerMsgCon;
    boost::signals2::connection playerTimeCon;
    // signals and slots end
    void setPlayerReallySendEOS(bool);
    bool getPlayerReallySendEOS();
    void buildInfoNode(BookInfoNode* info);
    amis::DaisyHandler *dh;
    Narrator *narrator;
    Player *player;
    bool bBookIsOpen;
    bool bUserAtEndOfBook;
    bool bContextMenuIsOpen;
    int lastReportedPlayerPosition;
    bool sectionIdxReportingEnabled;

    pthread_mutex_t *playerCallbackMutex;
    bool bPlayerReallySendEOS;

    void postBookData();
    Handle_JumpToSecond* jumpHandler1;

};

#endif
