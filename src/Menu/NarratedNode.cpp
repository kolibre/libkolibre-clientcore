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

#include "NarratedNode.h"
#include "../Defines.h"

#include <Narrator.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr narratedNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.narratednode"));

NarratedNode::NarratedNode(const std::string& name)
{
    name_ = name;
    narratedStrings.push_back(name);
}

NarratedNode::NarratedNode(const std::string& name, int number)
{
    appendNarratedString(number);
    name_ = name;
    narratedStrings.push_back(name);

    std::ostringstream uri_from_anything;
    uri_from_anything << reinterpret_cast<long>(this);
    uri_ = uri_from_anything.str();
}

void NarratedNode::appendNarratedString(const std::string& append)
{
    narratedStrings.push_back(append);
}

void NarratedNode::appendNarratedString(const std::string& append, const std::string parameter, int value)
{
    narratedStrings.push_back(append);

    std::vector<NarratedObject_t> params;
    params.push_back(NarratedObject_t(parameter, value));

    int id = narratedStrings.size() - 1;
    parameters[id] = params;
}

void NarratedNode::appendNarratedString(int number)
{
    narratedStrings.push_back(_N("{number}"));

    std::vector<NarratedObject_t> params;
    params.push_back(NarratedObject_t("number", number));

    int id = narratedStrings.size() - 1;
    parameters[id] = params;
}

void NarratedNode::appendNarratedTime(int hour, int minute, int second)
{
    narratedStrings.push_back(_N("{hour} {hour12} {minute} {second} {ampm}"));

    std::vector<NarratedObject_t> params;

    int hour12;
    bool isAm = false;

    if (hour < 0)
        hour = 0;
    if (hour > 24)
        hour = 0;
    if (hour > 12)
    {
        hour12 = hour - 12;
        isAm = true;
    }
    else
        hour12 = hour;

    if (minute < 0)
        minute = 0;
    if (minute > 59)
        minute = 59;
    if (second < 0)
        second = 0;
    if (second > 59)
        second = 59;

    params.push_back(NarratedObject_t("minute", minute));
    params.push_back(NarratedObject_t("second", second));
    params.push_back(NarratedObject_t("hour", hour));
    params.push_back(NarratedObject_t("hour12", hour12));
    if (isAm)
        params.push_back(NarratedObject_t("ampm", "am"));
    else
        params.push_back(NarratedObject_t("ampm", "pm"));

    unsigned int appliesTo = narratedStrings.size() - 1;
    parameters[appliesTo] = params;
}

void NarratedNode::appendNarratedDuration(int hour, int minute, int second)
{
    if (hour > 0)
    {
        if (hour == 1)
            narratedStrings.push_back(_N("one hour"));
        else
        {
            narratedStrings.push_back(_N("{2} hours"));
            std::vector<NarratedObject_t> params;
            params.push_back(NarratedObject_t("2", hour));

            unsigned int appliesTo = narratedStrings.size() - 1;
            parameters[appliesTo] = params;
        }
        if (minute != 0 || second != 0)
            narratedStrings.push_back(_N("and"));
    }

    if (minute > 0)
    {
        if (minute == 1)
            narratedStrings.push_back(_N("one minute"));
        else
        {
            narratedStrings.push_back(_N("{2} minutes"));
            std::vector<NarratedObject_t> params;
            params.push_back(NarratedObject_t("2", minute));

            unsigned int appliesTo = narratedStrings.size() - 1;
            parameters[appliesTo] = params;
        }
        if (second != 0)
            narratedStrings.push_back(_N("and"));
    }

    if (second > 0)
    {
        if (second == 1)
            narratedStrings.push_back(_N("one second"));
        else
        {
            narratedStrings.push_back(_N("{2} seconds"));
            std::vector<NarratedObject_t> params;
            params.push_back(NarratedObject_t("2", second));

            unsigned int appliesTo = narratedStrings.size() - 1;
            parameters[appliesTo] = params;
        }
    }

    // if the duration is zero narrate it
    if (hour == 0 && minute == 0 && second == 0)
    {
        narratedStrings.push_back(_N("{2} seconds"));
        std::vector<NarratedObject_t> params;
        params.push_back(NarratedObject_t("2", 0));

        unsigned int appliesTo = narratedStrings.size() - 1;
        parameters[appliesTo] = params;
    }
}

void NarratedNode::appendNarratedYear(int year)
{
    narratedStrings.push_back(_N("{year}"));

    std::vector<NarratedObject_t> params;
    params.push_back(NarratedObject_t("year", year));

    unsigned int appliesTo = narratedStrings.size() - 1;
    parameters[appliesTo] = params;
}

void NarratedNode::appendNarratedDate(int day, int month, int year)
{
    narratedStrings.push_back(_N("{dayname} {date} of {month} {year} {yearnum}"));

    std::vector<NarratedObject_t> params;
    params.push_back(NarratedObject_t("date", day));
    params.push_back(NarratedObject_t("month", month));
    params.push_back(NarratedObject_t("year", year));
    params.push_back(NarratedObject_t("yearnum", year));

    // Calculate the day this date ocurred (http://users.aol.com/s6sj7gt/mikecal.htm)
    int daynum = (day += month < 3 ? year-- : year - 2, 23 * month / 9 + day + 4 + year / 4 - year / 100 + year / 400) % 7;
    params.push_back(NarratedObject_t("dayname", daynum));

    unsigned int appliesTo = narratedStrings.size() - 1;
    parameters[appliesTo] = params;
}

bool NarratedNode::onNarrate()
{
    for (std::vector<std::string>::size_type strNum = 0;
            strNum < narratedStrings.size(); ++strNum)
    {
        std::vector<NarratedObject_t> params = parameters[strNum];
        for (std::vector<NarratedObject_t>::size_type pNum = 0;
                pNum < params.size(); ++pNum)
        {
            NarratedObject_t no = params[pNum];
            LOG4CXX_DEBUG(narratedNodeLog, "Setting parameter " << no.mKey << " of " << narratedStrings[strNum]);
            if (no.mStrValue != "")
                Narrator::Instance()->setParameter(no.mKey, no.mStrValue);
            else
                Narrator::Instance()->setParameter(no.mKey, no.mIntValue);
        }
        if (params.empty())
            LOG4CXX_DEBUG(narratedNodeLog, "No parameters for " << narratedStrings[strNum]);
        Narrator::Instance()->play(narratedStrings[strNum].c_str());
    }
    return true;
}

bool NarratedNode::onRender()
{
    return true;
}

bool NarratedNode::isVirtual()
{
    return false;
}
