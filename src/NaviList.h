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

#ifndef NAVILIST_H
#define NAVILIST_H

#include "NaviListItem.h"

#include <string>
#include <vector>

/**
 * A data type to hold information about a navigation list
 */
struct NaviList
{
    /**
     * The unique uri for this list
     */
    std::string uri_;

    /**
     * The name for this list
     */
    std::string name_;

    /**
     * The short description for this list
     */
    std::string short_;

    /**
     * Additional info about this list
     */
    std::string info_;

    /**
     * Vector containing navigation list items
     */
    std::vector<NaviListItem> items;

    NaviList() : uri_(""), name_(""), short_(""), info_("") {}
    NaviList(std::string name, std::string info) : uri_(""), name_(name), short_(""), info_(info){}

    virtual ~NaviList() {};
};
#endif
