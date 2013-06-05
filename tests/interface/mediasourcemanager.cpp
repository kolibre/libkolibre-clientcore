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
#include "MediaSourceManager.h"

int main(void)
{
    // setup logging
    setup_logging();
    logger->setLevel(log4cxx::Level::getTrace());

    MediaSourceManager *manager = MediaSourceManager::Instance();

    // add and remove daisy online services
    assert(manager->removeDaisyOnlineService(-1) == false);
    assert(manager->removeDaisyOnlineService(0) == false);
    assert(manager->getDaisyOnlineServiceIndex("") == -1);
    assert(manager->getDaisyOnlineServices() == 0);
    assert(manager->addDaisyOnlineService("", "url", "username", "password") == -1);
    assert(manager->addDaisyOnlineService("name", "", "username", "password") == -1);
    assert(manager->getDaisyOnlineServices() == 0);
    assert(manager->addDaisyOnlineService("name", "url", "username", "password") == 0);
    assert(manager->getDaisyOnlineServices() == 1);
    assert(manager->getDaisyOnlineServiceIndex("name") == 0);
    assert(manager->addDaisyOnlineService("name", "url", "username", "password") == -1);
    assert(manager->getDaisyOnlineServices() == 1);
    assert(manager->addDaisyOnlineService("foo", "bar", "username", "password") == 1);
    assert(manager->getDaisyOnlineServices() == 2);
    assert(manager->getDaisyOnlineServiceIndex("foo") == 1);
    assert(manager->addDaisyOnlineService("bar", "foo", "username", "password") == 2);
    assert(manager->getDaisyOnlineServices() == 3);
    assert(manager->getDaisyOnlineServiceIndex("bar") == 2);
    assert(manager->removeDaisyOnlineService(1));
    assert(manager->getDaisyOnlineServices() == 2);
    assert(manager->getDaisyOnlineServiceIndex("name") == 0);
    assert(manager->getDaisyOnlineServiceIndex("foo") == -1);
    assert(manager->getDaisyOnlineServiceIndex("bar") == 1);
    assert(manager->removeDaisyOnlineService(0));
    assert(manager->getDaisyOnlineServiceIndex("name") == -1);
    assert(manager->getDaisyOnlineServiceIndex("bar") == 0);
    assert(manager->removeDaisyOnlineService(1) == false);
    assert(manager->clearDaisyOnlineServies());
    assert(manager->getDaisyOnlineServices() == 0);

    // get and set daisy online service name
    assert(manager->setDOSname(-1, "foo") == false);
    assert(manager->setDOSname(0, "foo") == false);
    assert(manager->setDOSname(1, "foo") == false);
    manager->addDaisyOnlineService("foo", "bar", "username", "password");
    manager->addDaisyOnlineService("bar", "foo", "username", "password");
    assert(manager->getDaisyOnlineServiceIndex("foo") == 0);
    assert(manager->getDOSname(0) == "foo");
    assert(manager->setDOSname(0, "") == false);
    assert(manager->setDOSname(0, "bar") == false);
    assert(manager->setDOSname(0, "foobar"));
    assert(manager->getDOSname(0) == "foobar");
    assert(manager->getDaisyOnlineServiceIndex("foobar") == 0);
    assert(manager->getDaisyOnlineServiceIndex("bar") == 1);
    assert(manager->getDOSname(1) == "bar");
    assert(manager->setDOSname(1, "") == false);
    assert(manager->setDOSname(1, "foobar") == false);
    assert(manager->setDOSname(1, "foo"));
    assert(manager->getDaisyOnlineServiceIndex("foo") == 1);
    assert(manager->getDOSname(1) == "foo");
    manager->clearDaisyOnlineServies();

    // get and set daisy online service url
    assert(manager->setDOSurl(-1, "foo") == false);
    assert(manager->setDOSurl(0, "foo") == false);
    assert(manager->setDOSurl(1, "foo") == false);
    manager->addDaisyOnlineService("foo", "bar", "username", "password");
    manager->addDaisyOnlineService("bar", "foo", "username", "password");
    assert(manager->getDOSurl(0) == "bar");
    assert(manager->setDOSurl(0, "") == false);
    assert(manager->setDOSurl(0, "foo"));
    assert(manager->getDOSurl(0) == "foo");
    assert(manager->getDOSurl(1) == "foo");
    assert(manager->setDOSurl(1, "") == false);
    assert(manager->setDOSurl(1, "bar"));
    assert(manager->getDOSurl(1) == "bar");
    manager->clearDaisyOnlineServies();

    // get and set daisy online service username
    assert(manager->setDOSusername(-1, "foo") == false);
    assert(manager->setDOSusername(0, "foo") == false);
    assert(manager->setDOSusername(1, "foo") == false);
    manager->addDaisyOnlineService("foo", "bar", "username", "password");
    manager->addDaisyOnlineService("bar", "foo", "username", "password");
    assert(manager->getDOSusername(0) == "username");
    assert(manager->setDOSusername(0, "") == false);
    assert(manager->setDOSusername(0, "foo"));
    assert(manager->getDOSusername(0) == "foo");
    assert(manager->getDOSusername(1) == "username");
    assert(manager->setDOSusername(1, "") == false);
    assert(manager->setDOSusername(1, "bar"));
    assert(manager->getDOSusername(1) == "bar");
    manager->clearDaisyOnlineServies();

    // get and set daisy online service password
    assert(manager->setDOSpassword(-1, "foo") == false);
    assert(manager->setDOSpassword(0, "foo") == false);
    assert(manager->setDOSpassword(1, "foo") == false);
    manager->addDaisyOnlineService("foo", "bar", "username", "password");
    manager->addDaisyOnlineService("bar", "foo", "username", "password");
    assert(manager->getDOSpassword(0) == "password");
    assert(manager->setDOSpassword(0, "") == false);
    assert(manager->setDOSpassword(0, "foo"));
    assert(manager->getDOSpassword(0) == "foo");
    assert(manager->getDOSpassword(1) == "password");
    assert(manager->setDOSpassword(1, "") == false);
    assert(manager->setDOSpassword(1, "bar"));
    assert(manager->getDOSpassword(1) == "bar");
    manager->clearDaisyOnlineServies();

    // get and set daisy online service remember
    assert(manager->setDOSremember(-1, true) == false);
    assert(manager->setDOSremember(0, true) == false);
    assert(manager->setDOSremember(1, true) == false);
    manager->addDaisyOnlineService("foo", "bar", "username", "password");
    manager->addDaisyOnlineService("bar", "foo", "username", "password", true);
    assert(manager->getDOSremember(0) == false);
    assert(manager->setDOSremember(0, true));
    assert(manager->getDOSremember(0) == true);
    assert(manager->setDOSremember(0, false));
    assert(manager->getDOSremember(0) == false);
    assert(manager->getDOSremember(1) == true);
    assert(manager->setDOSremember(1, false));
    assert(manager->getDOSremember(1) == false);
    assert(manager->setDOSremember(1, true));
    assert(manager->getDOSremember(1) == true);
    manager->clearDaisyOnlineServies();

    // add and remove file system paths
    assert(manager->removeFileSystemPath(-1) == false);
    assert(manager->removeFileSystemPath(0) == false);
    assert(manager->getFileSystemPaths() == 0);
    assert(manager->getFileSystemPathIndex("") == -1);
    assert(manager->addFileSystemPath("", "path") == -1);
    assert(manager->addFileSystemPath("name", "") == -1);
    assert(manager->getFileSystemPaths() == 0);
    assert(manager->addFileSystemPath("name", "path") == 0);
    assert(manager->getFileSystemPaths() == 1);
    assert(manager->getFileSystemPathIndex("name") == 0);
    assert(manager->addFileSystemPath("name", "path") == -1);
    assert(manager->getFileSystemPaths() == 1);
    assert(manager->addFileSystemPath("foo", "bar") == 1);
    assert(manager->getFileSystemPaths() == 2);
    assert(manager->getFileSystemPathIndex("foo") == 1);
    assert(manager->addFileSystemPath("bar", "foo") == 2);
    assert(manager->getFileSystemPaths() == 3);
    assert(manager->getFileSystemPathIndex("bar") == 2);
    assert(manager->removeFileSystemPath(1));
    assert(manager->getFileSystemPaths() == 2);
    assert(manager->getFileSystemPathIndex("name") == 0);
    assert(manager->getFileSystemPathIndex("foo") == -1);
    assert(manager->getFileSystemPathIndex("bar") == 1);
    assert(manager->removeFileSystemPath(0));
    assert(manager->getFileSystemPathIndex("name") == -1);
    assert(manager->getFileSystemPathIndex("bar") == 0);
    assert(manager->removeFileSystemPath(1) == false);
    assert(manager->clearFileSystemsPaths());
    assert(manager->getFileSystemPaths() == 0);

    // get and set file system path name
    assert(manager->setFSPname(-1, "foo") == false);
    assert(manager->setFSPname(0, "foo") == false);
    assert(manager->setFSPname(1, "foo") == false);
    manager->addFileSystemPath("foo", "bar");
    manager->addFileSystemPath("bar", "foo");
    assert(manager->getFileSystemPathIndex("foo") == 0);
    assert(manager->getFSPname(0) == "foo");
    assert(manager->setFSPname(0, "") == false);
    assert(manager->setFSPname(0, "bar") == false);
    assert(manager->setFSPname(0, "foobar"));
    assert(manager->getFSPname(0) == "foobar");
    assert(manager->getFileSystemPathIndex("foobar") == 0);
    assert(manager->getFileSystemPathIndex("bar") == 1);
    assert(manager->getFSPname(1) == "bar");
    assert(manager->setFSPname(1, "") == false);
    assert(manager->setFSPname(1, "foobar") == false);
    assert(manager->setFSPname(1, "foo"));
    assert(manager->getFileSystemPathIndex("foo") == 1);
    assert(manager->getFSPname(1) == "foo");
    manager->clearFileSystemsPaths();

    // get and set file system path path
    assert(manager->setFSPpath(-1, "foo") == false);
    assert(manager->setFSPpath(0, "foo") == false);
    assert(manager->setFSPpath(1, "foo") == false);
    manager->addFileSystemPath("foo", "bar");
    manager->addFileSystemPath("bar", "foo");
    assert(manager->getFSPpath(0) == "bar");
    assert(manager->setFSPpath(0, "") == false);
    assert(manager->setFSPpath(0, "foo"));
    assert(manager->getFSPpath(0) == "foo");
    assert(manager->getFSPpath(1) == "foo");
    assert(manager->setFSPpath(1, "") == false);
    assert(manager->setFSPpath(1, "bar"));
    assert(manager->getFSPpath(1) == "bar");
    manager->clearFileSystemsPaths();

    manager->DeleteInstance();

    return 0;
}
