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

#ifndef NAVILIST_ITEM_H
#define NAVILIST_ITEM_H

#include <string>

/**
 * A data type to hold information about a navigation list item
 */
struct NaviListItem
{
    /**
     * The unique uri for this item
     */
    std::string uri_;

    /**
     * The name for this this item
     */
    std::string name_;

    /**
     * Additional info about this item
     */
    std::string info_;

    NaviListItem() : uri_(""), name_(""), info_("") {}
    NaviListItem(std::string uri, std::string name) : uri_(uri), name_(name), info_("") {}
    virtual ~NaviListItem() {};
};
#endif
