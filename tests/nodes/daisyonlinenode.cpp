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

#include "DaisyOnlineNode.h"
#include "CommandQueue2/CommandQueue.h"
#include "Commands/NotifyCommands.h"
#include "Commands/InternalCommands.h"
#include "MediaSourceManager.h"
#include "../setup_logging.h"

#include <NaviEngine.h>

#include <assert.h>
#include <cstdlib>
#include <iostream>

using namespace std;
bool running = true;
bool notifyLoginOkReceived = false;
bool notifyLoginFailReceived = false;

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

void* dispatchThread(void*)
{
    while (running)
    {
        cq2::Dispatcher::instance().dispatchCommand();
    }
    pthread_exit(NULL);
}

struct Handle_NotifyCommands: public cq2::Handler<NOTIFY_COMMAND>
{
    Handle_NotifyCommands() {}

private:
    void handle(NOTIFY_COMMAND command)
    {
        switch (command)
        {
        case NOTIFY_LOGIN_OK:
            notifyLoginOkReceived = true;
            break;
        case NOTIFY_LOGIN_FAIL:
            notifyLoginFailReceived = true;
            break;
        }
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

    // setup command handlers and dispatch thread
    Handle_NotifyCommands notifyHandler;
    notifyHandler.listen();
    pthread_t tdispatch;
    pthread_create(&tdispatch, NULL, dispatchThread, NULL);

    // add DaisyOnliceService source with incorrect username and password
    MediaSourceManager::Instance()->addDaisyOnlineService("localhost", argv[1], "incorrect", "incorrect");

    Navi navi;
    DaisyOnlineNode::errorType error = (DaisyOnlineNode::errorType)-1;
    DaisyOnlineNode *node = new DaisyOnlineNode("localhost", argv[1], "", "", ".", "");
    navi.openMenu(node, false);

    // open should fail with incorrect username and password
    assert(node->onOpen(navi));
    error = node->getLastError();
    assert(error == DaisyOnlineNode::USERNAME_PASSWORD_ERROR);
    sleep(1);
    assert(notifyLoginFailReceived == true);

    // a normal login retry without changing username or password should not trigger in invoke
    // of logOn nor the NOTIFY_LOGIN_FAIL command
    notifyLoginFailReceived = false;
    node->process(navi, COMMAND_RETRY_LOGIN);
    error = node->getLastError();
    assert(error == DaisyOnlineNode::USERNAME_PASSWORD_ERROR);
    sleep(1);
    assert(notifyLoginFailReceived == false);

    // a forced login retry should have the opposite effect as a normal login retry
    notifyLoginFailReceived = false;
    node->process(navi, COMMAND_RETRY_LOGIN_FORCED);
    error = node->getLastError();
    assert(error == DaisyOnlineNode::USERNAME_PASSWORD_ERROR);
    sleep(1);
    assert(notifyLoginFailReceived == true);

    // changing password should also have the opposite effect
    MediaSourceManager::Instance()->setDOSpassword(0, "correct");
    notifyLoginFailReceived = false;
    node->process(navi, COMMAND_RETRY_LOGIN);
    error = node->getLastError();
    assert(error == DaisyOnlineNode::USERNAME_PASSWORD_ERROR);
    sleep(1);
    assert(notifyLoginFailReceived == true);

    // changing username should also have the opposite effect, but session init fails
    // when invoking getServiceAttributes due to soap fault
    MediaSourceManager::Instance()->setDOSusername(0, "correct");
    node->process(navi, COMMAND_RETRY_LOGIN);
    error = node->getLastError();
    assert(error == DaisyOnlineNode::SERVICE_ERROR);

    // next session init fails when invoking setReadingsSystem attributes due to soap fault
    node->process(navi, COMMAND_RETRY_LOGIN_FORCED);
    error = node->getLastError();
    assert(error == DaisyOnlineNode::SERVICE_ERROR);

    // next we establish a session and and issue new content
    node->process(navi, COMMAND_RETRY_LOGIN_FORCED);
    error = node->getLastError();
    assert(error == DaisyOnlineNode::OK);
    sleep(1);
    assert(notifyLoginOkReceived == true);

    // finally we retrieve a content list of issued content
    node->process(navi, COMMAND_DO_GETCONTENTLIST);
    error = node->getLastError();
    assert(error == DaisyOnlineNode::OK);

    // a sleep here will prevent random segmentation faults in narrator thread
    running = false;
    pthread_join(tdispatch, NULL);
    sleep(1);

    return 0;
}
