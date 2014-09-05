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

#ifndef NAVIENGINE_SLEEPTIMERNODE
#define NAVIENGINE_SLEEPTIMERNODE

#include "VirtualContextMenuNode.h"

#include <map>
#include <string>

class ClientCore;

class SleepTimerNode: public VirtualContextMenuNode
{
public:
    SleepTimerNode(ClientCore*, const std::string&, const std::string&);

    bool up(naviengine::NaviEngine&);
    bool prev(naviengine::NaviEngine&);
    bool next(naviengine::NaviEngine&);
    bool select(naviengine::NaviEngine&);
    bool selectByUri(naviengine::NaviEngine&, std::string);
    bool onOpen(naviengine::NaviEngine&);
    bool narrateInfo();
    bool onNarrate();

private:
    void render();
    void renderChild();

    bool isOpen;
    ClientCore* clientcore_;
    std::map<int, int> states;
};

#endif
