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

#include "BookInfoNode.h"
#include "config.h"
#include "../Commands/InternalCommands.h"

#include <NaviEngine.h>
#include <Narrator.h>

using namespace naviengine;

BookInfoNode::BookInfoNode(const std::string& name, const std::string& playBeforeOnOpen) :
        ContextMenuNode(name, playBeforeOnOpen)
{
}

bool BookInfoNode::select(NaviEngine&)
{
    return false;
}

bool BookInfoNode::onOpen(NaviEngine&)
{
    Narrator::Instance()->setPushCommandFinished(true);
    return true;
}

bool BookInfoNode::process(NaviEngine& navi, int command, void* data)
{
    switch (command)
    {
    case COMMAND_NARRATORFINISHED:
        if (navi.getCurrentChoice() != 0 and lastChild() != navi.getCurrentChoice())
        {
            return next(navi);
        }
        else
        {
            // If all the nodes are read, return control to parent
            Narrator::Instance()->setPushCommandFinished(false); // Let the parent enable callbacks if he needs them.
            return MenuNode::up(navi);
        }
        break;

    default:
        break;
    }
    return false;
}
