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

/**
 * NaviListItem and NaviList implementations
 *
 * The implementation for building items and lists-of-items is hidden
 * in these impl classes in order to keep the intrfaces clean and the
 * implementation of NaviEngine less cluttered.
 *
 */
#include "NaviList.h"
#include "NaviListItem.h"

#ifndef NAVILISTIMPL_H
#define NAVILISTIMPL_H

namespace naviengine
{
class AnyNode;
class MenuNode;

struct NaviListItemImpl: public NaviListItem
{
    NaviListItemImpl(AnyNode const*);
    ~NaviListItemImpl();
};

struct NaviListImpl: public NaviList
{
    NaviListImpl(AnyNode const*);
    ~NaviListImpl();
};

}
#endif
