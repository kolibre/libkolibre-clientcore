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

#ifndef _NAVI_H
#define _NAVI_H

#include <NaviEngine.h>

#include <string>

// forward declare data types
namespace naviengine
{
class MenuNode;
}
class ClientCore;

/**
 * Navi implements NaviEngine, this enables ClientCore.cpp to use Navi
 * like before the introduction of NaviEngine, and at the same time
 * Navi gains access to the menumodel implementation.
 */
class Navi: public naviengine::NaviEngine
{
public:
    Navi(ClientCore*);
    ~Navi();

    // Process a command
    bool process(int command, void* data = 0);

    // Build context menu
    naviengine::MenuNode* buildContextMenu();
private:
    void narrateInfoForCurrentNode();
    void narrateChange(const MenuState& before, const MenuState& after);
    void narrate(const std::string text);
    void narrate(const int value);
    void narrateStop();
    void narrateShortPause();
    void narrateLongPause();
    void narrateSetParam(std::string param, int number);

    ClientCore* clientcore_;
};

#endif
