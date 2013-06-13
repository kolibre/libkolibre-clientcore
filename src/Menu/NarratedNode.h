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

#ifndef NAVIENGINE_NARRATEDNODE
#define NAVIENGINE_NARRATEDNODE

#include <Nodes/AnyNode.h>

#include <vector>
#include <map>
#include <string>

struct NarratedObject_t
{
    std::string mKey;
    int mIntValue;
    std::string mStrValue;
    NarratedObject_t(std::string key, int value) : mKey(key), mIntValue(value), mStrValue("") {}
    NarratedObject_t(std::string key, std::string value) : mKey(key), mIntValue(0), mStrValue(value) {}
};

class NarratedNode: public naviengine::AnyNode
{
public:
    NarratedNode(const std::string& name);
    NarratedNode(const std::string& name, int number);
    void appendNarratedString(const std::string& append);
    void appendNarratedString(const std::string& append, const std::string parameter, int value);
    void appendNarratedString(int number);
    void appendNarratedTime(int hour, int minute, int second);
    void appendNarratedDuration(int hour, int minute, int second);
    void appendNarratedYear(int year);
    void appendNarratedDate(int day, int month, int year);

    // It does not handle input
    virtual bool select(naviengine::NaviEngine&)
    {
        return false; // Does not have children to select
    }
    virtual bool selectByUri(naviengine::NaviEngine&, std::string)
    {
        return false; // Does not have children to select
    }
    virtual bool up(naviengine::NaviEngine&)
    {
        return false; // Will never be opened so up is unnecessary.
    }
    virtual bool next(naviengine::NaviEngine&)
    {
        return false; // Does not have children to iterate
    }
    virtual bool prev(naviengine::NaviEngine&)
    {
        return false; // Does not have children to iterate
    }
    virtual bool menu(naviengine::NaviEngine&)
    {
        return false; // no function
    }
    virtual bool process(naviengine::NaviEngine&, int, void*)
    {
        return false; // Will never be opened so process is unnecessary.
    }
    virtual bool onOpen(naviengine::NaviEngine& navi)
    {
        return false; // Should never be opened
    }
    virtual void beforeOnOpen()
    {
        // Should never be opened
    }

    // Always narrated by itself.
    virtual bool onNarrate();
    virtual bool onRender();
    virtual bool isVirtual();
private:
    std::vector<std::string> narratedStrings;
    std::map<int, std::vector<NarratedObject_t> > parameters;
};

#endif
