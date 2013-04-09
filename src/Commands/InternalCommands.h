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

#ifndef COMMANDS_INTERNAL_H
#define COMMANDS_INTERNAL_H

enum INTERNAL_COMMAND
{

    COMMAND_NONE,

    /***********************************
     * USER INTERACTION
     ***********************************/

    /*
     * KEY PRESSES
     */

    COMMAND_HOME,
    COMMAND_BACK,
    COMMAND_PAUSE,
    COMMAND_UP,
    COMMAND_DOWN,
    COMMAND_LEFT,
    COMMAND_RIGHT,
    COMMAND_BOOKMARK,
    /*
     * MENU COMMANDS
     */

    COMMAND_OPEN_CONTEXTMENU,
    COMMAND_OPEN_BOOKINFO,
    COMMAND_OPEN_MENU_GOTOTIMENODE,
    COMMAND_OPEN_MENU_GOTOPERCENTNODE,
    COMMAND_OPEN_MENU_GOTOPAGENODE,

    /***********************************
     * BACKEND COMMANDS
     ***********************************/

    /*
     * AUTOMATIC COMMANDS
     */

    COMMAND_NEXT,
    COMMAND_LAST,
    COMMAND_INFO,
    COMMAND_NARRATORFINISHED,
    COMMAND_DO_GETCONTENTLIST,

    /*
     * LOGIN COMMANDS
     */

    COMMAND_RETRY_LOGIN,
    COMMAND_RETRY_LOGIN_FORCED,
};

#endif
