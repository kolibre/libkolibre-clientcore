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

#include <stdlib.h>
#include <assert.h>

using namespace std;

#include "../setup_logging.h"
#include "Settings.h"

int main(void)
{

    char data_path_variable[] = "KOLIBRE_DATA_PATH";
    char data_path_value[] = ".";
    setenv(data_path_variable, data_path_value, true);

    //Setup logging
    setup_logging();
    Settings *settings = Settings::Instance();

    /*
     * test read<T> with different types
     */

    int integerValue = 0;
    integerValue = settings->read<int>("integerValue", 13);
    assert(integerValue == 13);

    double doubleValue = 0;
    doubleValue = settings->read<double>("doubleValue", 1.8);
    assert(doubleValue == 1.8);

    bool boolValue = false;
    boolValue = settings->read<bool>("boolValue", false);
    assert(boolValue == false);

    string stringValue = "foo";
    stringValue = settings->read<string>("stringValue", "bar");
    assert(stringValue == "bar");

    /*
     * test write<T> and read<T> with different types
     */

    settings->write<int>("integerValue", 13);
    integerValue = 0;
    integerValue = settings->read<int>("integerValue");
    assert(integerValue == 13);

    settings->write<double>("doubleValue", 1.8);
    doubleValue = 0;
    doubleValue = settings->read<double>("doubleValue");
    assert(doubleValue == 1.8);

    settings->write<bool>("boolValue", false);
    boolValue = false;
    boolValue = settings->read<bool>("boolValue");
    assert(boolValue == false);

    settings->write<string>("stringValue", "bar");
    stringValue = "foo";
    stringValue = settings->read<string>("stringValue");
    assert(stringValue == "bar");

    /*
     * test readInto<T> with different types
     */

    integerValue = 0;
    settings->readInto<int>(integerValue, "integerValue");
    assert(integerValue == 13);

    doubleValue = 0;
    settings->readInto<double>(doubleValue, "doubleValue");
    assert(doubleValue == 1.8);

    boolValue = false;
    settings->readInto<bool>(boolValue, "boolValue");
    assert(boolValue == false);

    stringValue = "foo";
    settings->readInto<string>(stringValue, "stringValue");
    assert(stringValue == "bar");

    delete settings;

    return 0;
}
