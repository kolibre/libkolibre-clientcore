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

#ifndef _MP3NODE_H
#define _MP3NODE_H

#include "NaviList.h"

#include <Nodes/VirtualMenuNode.h>

#include <map>
#include <string>
#include <boost/signals2.hpp>

/**
 * MP3Node implements the VirtualMenuNode, making audio files traversable
 */
class MP3Node: public naviengine::VirtualMenuNode
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
    bool select(naviengine::NaviEngine&);
    bool selectByUri(naviengine::NaviEngine&, std::string);
    bool abort();
    bool narrateName();
    bool narrateInfo();
    bool onNarrate();
    bool onRender();
    void onNarratorDone();

private:
    bool pathUpdated_;
    bool openFirstChild_;
    boost::signals2::connection narratorDoneConnection;

    std::string fsName_;
    std::string fsPath_;

    std::map<int, std::string> files;

    void announce();
    void announceSelection();
};

#endif
