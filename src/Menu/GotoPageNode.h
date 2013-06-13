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

#ifndef GOTOPAGENODE_H_
#define GOTOPAGENODE_H_

#include <NaviEngine.h>
#include <Nodes/VirtualMenuNode.h>

#include <map>
#include <string>

class DaisyNavi;

class GotoPageNode: public naviengine::VirtualMenuNode
{
public:
    GotoPageNode(int max, DaisyNavi*);
    bool up(naviengine::NaviEngine&);
    bool prev(naviengine::NaviEngine&);
    bool next(naviengine::NaviEngine&);
    bool select(naviengine::NaviEngine&);
    bool selectByUri(naviengine::NaviEngine&, std::string);
    bool onOpen(naviengine::NaviEngine&);
    void beforeOnOpen();
    bool onNarrate();

private:
    void render();
    void renderChild(bool silent = false);
    void narrateValue(long aValue, bool bNarrateMax);
    void narrateEnterType();

    enum actionCode
    {
        NARRATE_TYPE,
        NARRATE_GOTO,
        NARRATE_TYPES,
        NARRATE_GOINGTO,
        NARRATE_NUMBERSIZE,
        NARRATE_HOURS,
        NARRATE_MINUTES,
        NARRATE_SECONDS,
        NARRATE_ENTER,
    };

    std::map<actionCode, std::string> mapNarrations;

    bool isOpen;
    int iMax;
    DaisyNavi* daisyNavi;
};

#endif /* GOTOPAGENODE_H_ */
