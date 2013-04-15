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

#include "DaisyOnlineBookNode.h"
#include "../setup_logging.h"

#include <NaviEngine.h>
#include <DaisyOnlineHandler.h>

#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

using namespace std;

class Navi: public naviengine::NaviEngine
{
    naviengine::MenuNode* buildContextMenu()
    {
        return NULL;
    }

    void narrateChange(const naviengine::NaviEngine::MenuState& before, const naviengine::NaviEngine::MenuState& after)
    {
    }

    void narrate(std::string message)
    {
    }

    void narrate(int value)
    {
    }

    void narrateStop()
    {
    }

    void narrateShortPause()
    {
    }

    void narrateLongPause()
    {
    }
};

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <uri>" << std::endl;
        return -1;
    }

    if (getenv("AXIS2_HOME"))
    {
        std::cout << "Setting AXIS2_HOME forces logs to be written there, so don't!" << std::endl;
        return -1;
    }

    // setup logging
    setup_logging();

    Navi navi;

    DaisyOnlineBookNode::errorType error = (DaisyOnlineBookNode::errorType)-1;
    DaisyOnlineHandler DOHandler(argv[1]);
    DaisyOnlineBookNode node("any_id", &DOHandler);

    // Note: onOpen fails when no server is hosting the content resources. i.e. no file server

    /*
     * Test 1: ncc file is found in content resources
     */
    assert(not node.onOpen(navi));
    error = node.getLastError();
    assert(error == DaisyOnlineBookNode::OPEN_CONTENT_ERROR);

    /*
     * Test 2: ncc file is not found in content resources
     */
    assert(not node.onOpen(navi));
    error = node.getLastError();
    assert(error == DaisyOnlineBookNode::NAVIGATION_CONTROL_FILE_ERROR);

    /*
     * Test 3: content resources failed due to soap fault
     */
    assert(not node.onOpen(navi));
    error = node.getLastError();
    assert(error == DaisyOnlineBookNode::DO_INVOKE_ERROR);

    // a sleep here will prevent random segmentation faults in narrator thread
    sleep(1);

    return 0;
}
