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

#ifndef _ROOTNODE_H
#define _ROOTNODE_H

#include "NaviList.h"

#include <Nodes/MenuNode.h>

#include <string>

/**
 * RootNode implements the MenuNode, making available media sources function like a menu.
 */
class RootNode: public naviengine::MenuNode
{
public:
    RootNode(const std::string useragent = "");
    ~RootNode();

    // MenuNode start
    // virtual methods from naviengine::AnyNode
    bool menu(naviengine::NaviEngine&);
    bool onOpen(naviengine::NaviEngine&);
    bool process(naviengine::NaviEngine&, int command, void* data = 0);
    bool next(naviengine::NaviEngine&);
    bool prev(naviengine::NaviEngine&);
    bool up(naviengine::NaviEngine&);
    bool narrateInfo();
    bool onNarrate();
    bool onRender();

private:
    NaviList navilist_;
    AnyNode* currentChild_;

    std::string userAgent_;

    void announce();
    void announceSelection();
};

#endif
