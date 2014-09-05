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

#include "ContextMenuNode.h"
#include "../Defines.h"

#include <Narrator.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr contextMenuNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.contextmenunode"));

ContextMenuNode::ContextMenuNode(const std::string& name, const std::string& playBeforeOnOpen) : MenuNode(name)
{
    playBeforeOnOpen_ = playBeforeOnOpen;
    info_ = _N("choose option using left and right arrows, open using play button");
}

void ContextMenuNode::beforeOnOpen()
{
    Narrator::Instance()->play(playBeforeOnOpen_.c_str());
}
