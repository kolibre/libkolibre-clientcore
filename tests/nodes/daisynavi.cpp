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

#include "DaisyNavi.h"
#include "CommandQueue2/CommandQueue.h"
#include "Commands/InternalCommands.h"
#include "../setup_logging.h"

#include <NaviEngine.h>

#include <assert.h>
#include <cstdlib>
#include <iostream>

using namespace std;
bool running = true;

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

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " /path/to/navigation/file" << std::endl;
        return -1;
    }

    // setup logging
    setup_logging();

    // setup command handlers and dispatch thread
    pthread_t tdispatch;
    pthread_create(&tdispatch, NULL, dispatchThread, NULL);

    // setup player
    Player *player = Player::Instance();
    player->enable(NULL, NULL);
    player->setTempo(1.0);

    Navi navi;
    DaisyNavi daisynavi;

    // open daisy book
    assert(daisynavi.open(argv[1]));
    assert(daisynavi.onOpen(navi));
    assert(daisynavi.isOpen());

    sleep(10);
    std::cout << "currentTime: " << daisynavi.getCurrentTime() << std::endl;
    std::cout << "currentPageIdx: " << daisynavi.getCurrentPageIdx() << std::endl;
    std::cout << "currentPercent: " << daisynavi.getCurrentPercent() << std::endl;
    //while (player->isPlaying()) usleep(10000);

    daisynavi.up(navi);
    for (int i=0; i<5; i++)
    {
        daisynavi.next(navi);
        std::cout << "currentTime: " << daisynavi.getCurrentTime() << std::endl;
        std::cout << "currentPageIdx: " << daisynavi.getCurrentPageIdx() << std::endl;
        std::cout << "currentPercent: " << daisynavi.getCurrentPercent() << std::endl;
        //daisynavi.process(navi, COMMAND_NARRATORFINISHED, NULL);
        sleep(5);
    }

    // close daisy book
    assert(daisynavi.closeBook());

    // test closing book by changing navi level
    // test going to next phrase
    // test jumping to sections
    // test jumping to percent
    // test resuming from lastmark

    // a sleep here will prevent random segmentation faults in narrator thread
    running = false;
    sleep(1);

    return 0;
}
