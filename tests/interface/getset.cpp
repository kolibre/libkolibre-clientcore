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

#include "ClientCore.h"
#include "../setup_logging.h"

#include <assert.h>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    // setup logging
    setup_logging();

    ClientCore clientcore("some url", "useragent");

    // add service
    int index = 0;
    index = clientcore.addDaisyOnlineService("", "", "username", "password");
    assert(index == -1);
    index = clientcore.addDaisyOnlineService("some url", "", "username", "password");
    assert(index == -1);
    index = clientcore.addDaisyOnlineService("some name", "some url", "username", "password");
    assert(index == 0);
    index = clientcore.addDaisyOnlineService("some name", "some other url", "username", "password");
    assert(index == -1);
    index = clientcore.addDaisyOnlineService("some other name", "some url", "username", "password");
    assert(index == 1);

    // add path
    index = clientcore.addFileSystemPath("", "");
    assert(index == -1);
    index = clientcore.addFileSystemPath("some name", "");
    assert(index == -1);
    index = clientcore.addFileSystemPath("some name", "some path");
    assert(index == 0);
    index = clientcore.addFileSystemPath("some name", "some other path");
    assert(index == -1);
    index = clientcore.addFileSystemPath("some other name", "some path");
    assert(index == 1);

    // getServiceUrl
    assert(clientcore.getServiceUrl() == "some url");

    // getUserAgent
    assert(clientcore.getUserAgent() == "useragent");

    // setManualSound
    clientcore.setManualSound("manual");
    assert(clientcore.getManualSound() == "manual");

    // setAboutSound
    clientcore.setAboutSound("about");
    assert(clientcore.getAboutSound() == "about");

    // setServiceUrl
    clientcore.setServiceUrl("url");
    assert(clientcore.getServiceUrl() == "url");

    // setUsername
    clientcore.setUsername("username");
    assert(clientcore.getUsername() == "username");

    // setPassword and getRememberPassword
    clientcore.setPassword("password", false);
    assert(clientcore.getPassword() == "password");
    assert(clientcore.getRememberPassword() == false);
    clientcore.setPassword("drowssap", true);
    assert(clientcore.getPassword() == "drowssap");
    assert(clientcore.getRememberPassword() == true);

    // setSerialNumber
    clientcore.setSerialNumber("serial");
    assert(clientcore.getSerialNumber() == "serial");

    // setLanguage
    clientcore.setLanguage("en");
    assert(clientcore.getLanguage() == "en");
    clientcore.setLanguage("en-EN");
    assert(clientcore.getLanguage() == "en");
    clientcore.setLanguage("en-ENG");
    assert(clientcore.getLanguage() == "en");
    clientcore.setLanguage("en-123");
    assert(clientcore.getLanguage() == "en");
    clientcore.setLanguage("EN");
    assert(clientcore.getLanguage() == "en");
    clientcore.setLanguage("english");
    assert(clientcore.getLanguage() == "en");
    clientcore.setLanguage("sv_SE");
    assert(clientcore.getLanguage() == "en");
    clientcore.setLanguage("sv-SWEDEN");
    assert(clientcore.getLanguage() == "en");

    return 0;
}
