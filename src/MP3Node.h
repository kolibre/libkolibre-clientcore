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

#ifndef _FILESYSTEMNODE_H
#define _FILESYSTEM_H

#include "NaviList.h"

#include <Nodes/MenuNode.h>

#include <string>
#include <boost/signals2.hpp>

/**
 * MP3Node implements the MenuNode, making available media types function like a menu.
 */
class MP3Node: public naviengine::MenuNode
{
public:
    MP3Node(const std::string name, const std::string path, bool openFirstChild = false);
    ~MP3Node();

    // MenuNode start
    // virtual methods from naviengine::AnyNode
    bool menu(naviengine::NaviEngine&);
    bool onOpen(naviengine::NaviEngine&);
    void beforeOnOpen();
    bool process(naviengine::NaviEngine&, int command, void* data = 0);
    bool next(naviengine::NaviEngine&);
    bool prev(naviengine::NaviEngine&);
    bool up(naviengine::NaviEngine&);
    bool abort();
    bool narrateName();
    bool narrateInfo();
    bool onNarrate();
    bool onRender();
    void onNarratorDone();

private:
    NaviList navilist_;
    AnyNode* currentChild_;
    bool pathUpdated_;
    bool openFirstChild_;
    boost::signals2::connection narratorDoneConnection;

    std::string fsName_;
    std::string fsPath_;

    void announce();
    void announceSelection();
};

#endif
