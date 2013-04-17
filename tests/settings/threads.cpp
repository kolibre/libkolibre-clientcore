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
#include <stdio.h>
#include <string.h>

using namespace std;

#include "../setup_logging.h"
#include "Settings.h"

void remove_file(const char* file)
{
    //Remove old settings databases
    if( remove( file ) != 0 )
        cout << "Settings db not created" << endl;
    else
        cout << "Settings db removed" << endl;
}

void *exerciseDatabase( void *ptr ){
    Settings *settings = Settings::Instance();

    assert(settings->write<int>("integerValue", 13));
    int integerValue = 0;
    integerValue = settings->read<int>("integerValue");
    assert(integerValue == 13);

    assert(settings->write<double>("doubleValue", 1.8));
    double doubleValue = 0;
    doubleValue = settings->read<double>("doubleValue");
    assert(doubleValue == 1.8);

    assert(settings->write<bool>("boolValue", false));
    bool boolValue = false;
    boolValue = settings->read<bool>("boolValue");
    assert(boolValue == false);

    assert(settings->write<string>("stringValue", "bar"));
    string stringValue = "foo";
    stringValue = settings->read<string>("stringValue");
    assert(stringValue == "bar");

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
}

int main(void)
{
    pthread_t thread1, thread2, thread3, thread4;
    string data_path_variable = "KOLIBRE_DATA_PATH";
    string data_path_value = ".";
    setenv(data_path_variable.c_str(), data_path_value.c_str(), true);

    //Setup logging
    setup_logging();
    logger->setLevel(log4cxx::Level::getTrace());
    cout << "1..2" << endl;
    cout << "#" << endl
        << "# Try reading and  writing to database from differetnt threadsry" << endl
        << "#" << endl;

    

    //Remove old settings databases
    remove_file( (data_path_value + "/settings.db").c_str() );

    Settings *settings = Settings::Instance();

    //Run threadsSetup threads
    pthread_create(&thread1, NULL, exerciseDatabase, NULL);
    pthread_create(&thread2, NULL, exerciseDatabase, NULL);
    pthread_create(&thread3, NULL, exerciseDatabase, NULL);
    pthread_create(&thread4, NULL, exerciseDatabase, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pthread_join(thread4, NULL);

    cout << "ok 1 - Read and write from threads" << endl;

    settings->DeleteInstance();
    settings=NULL;

    cout << "ok 2 - Stop database instance" << endl;

    //Cleanup
    remove_file( (data_path_value + "/settings.db").c_str() );

    return 0;
}
