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

#ifndef _DAISYBOOKNODE_H
#define _DAISYBOOKNODE_H

#include <Nodes/VirtualMenuNode.h>

#include <string>

class DaisyNavi;

class DaisyBookNode: public naviengine::VirtualMenuNode
{
public:
    DaisyBookNode();
    DaisyBookNode(std::string);
    ~DaisyBookNode();

    bool up(naviengine::NaviEngine&);
    bool prev(naviengine::NaviEngine&);
    bool next(naviengine::NaviEngine&);
    bool select(naviengine::NaviEngine&);
    bool selectByUri(naviengine::NaviEngine&, std::string);
    bool menu(naviengine::NaviEngine&);
    bool onOpen(naviengine::NaviEngine&);
    void beforeOnOpen();
    bool process(naviengine::NaviEngine&, int, void*);
    bool onNarrate();

protected:
    DaisyNavi *pDaisyNavi;
    bool daisyNaviActive;
    std::string daisyUri_;
};

#endif
