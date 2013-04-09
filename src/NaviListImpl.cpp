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

#include "NaviListImpl.h"
#include "Defines.h"
#include "config.h"

#include <Nodes/MenuNode.h>

#include <sstream>
#include <libintl.h>
#include <assert.h>

naviengine::NaviListItemImpl::NaviListItemImpl(AnyNode const* node)
{
    if (node == NULL)
    {
        assert(!"NaviListItems cannot be built from NULL");
        return;
    }
    name_ = _(node->name_.c_str());
    //info_ = _(node->info_.c_str());
    uri_ = node->uri_;

    if (uri_.empty())
    {
        std::ostringstream uri_from_anything;
        uri_from_anything << reinterpret_cast<long>(this);
        uri_ = uri_from_anything.str();
    }
}

naviengine::NaviListItemImpl::~NaviListItemImpl()
{
}

naviengine::NaviListImpl::NaviListImpl(AnyNode const* container)
{
    if (container == NULL)
    {
        assert(!"NaviLists cannot be built from NULL");
        return;
    }

    name_ = _(container->name_.c_str());
    info_ = _(container->info_.c_str());
    uri_ = container->uri_;

    if (uri_.empty())
    {
        std::ostringstream uri_from_anything;
        uri_from_anything << reinterpret_cast<long>(this);
        uri_ = uri_from_anything.str();
    }

    AnyNode* first = container->firstChild();
    if (first == NULL)
        return;

    AnyNode* child = first;

    do
    {
        items.push_back(NaviListItemImpl(child));
        child = child->next_;
    } while (child != NULL and child != first);
}

naviengine::NaviListImpl::~NaviListImpl()
{
}
