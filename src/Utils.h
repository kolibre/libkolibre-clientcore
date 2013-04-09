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

#ifndef _UTILS_H
#define _UTILS_H

#include <string>
#include <cstdlib>

#ifdef WIN32
#define DEFAULT_DATAPATH "./"
#define ENVIRONMENT_PATH "KOLIBRE_DATA_PATH_UTF8"
#define PATH_SEPARATOR_CHAR '\\'
#define PATH_SEPARATOR_STR "\\"
#else
#define DEFAULT_DATAPATH "/tmp/"
#define ENVIRONMENT_PATH "KOLIBRE_DATA_PATH"
#define PATH_SEPARATOR_CHAR '/'
#define PATH_SEPARATOR_STR "/"
#endif

class Utils
{
public:
    static void trim(std::string& s)
    {
        // Remove leading and trailing whitespace
        static const char whitespace[] = " \n\t\v\r\f";
        s.erase(0, s.find_first_not_of(whitespace));
        s.erase(s.find_last_not_of(whitespace) + 1U);
    }

    static std::string getDatapath()
    {
        char *path;
        path = getenv(ENVIRONMENT_PATH);
        if (path == NULL)
            return DEFAULT_DATAPATH;

        std::string datapath = path;
        if (datapath.find_last_of(PATH_SEPARATOR_CHAR) != datapath.length() - 1)
        {
            datapath.append(PATH_SEPARATOR_STR);
        }

        return datapath;
    }

private:
};

#endif
