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
#include "CommandQueue2/CommandQueue.h"
#include "Commands/NotifyCommands.h"
#include "../setup_logging.h"

#include <assert.h>
#include <cstdlib>
#include <iostream>

using namespace std;

bool notifySleepNearTimeoutReceived = false;
bool notifySleepTimeoutReceived = false;

void printSleepTimerInfo(ClientCore& clientcore)
{
    cout << "timeleft: " << clientcore.getSleepTimerTimeLeft() << endl;
    cout << "setting: " << clientcore.getSleepTimerSetting() << endl;
    switch (clientcore.getSleepTimerState())
    {
        case SLEEP_TIMER_OFF:
            cout << "state: SLEEP_TIMER_OFF" << endl;
            break;
        case SLEEP_TIMER_TIMED_OUT:
            cout << "state : SLEEP_TIMER_TIMED_OUT" << endl;
            break;
        case SLEEP_TIMER_NEAR_TIMEOUT:
            cout << "state : SLEEP_TIMER_NEAR_TIMEOUT" << endl;
            break;
        case SLEEP_TIMER_ON:
            cout << "state : SLEEP_TIMER_ON" << endl;
            break;
    }
}

struct Handle_NotifyCommands: public cq2::Handler<NOTIFY_COMMAND>
{
    Handle_NotifyCommands() {}

private:

    void handle(NOTIFY_COMMAND command)
    {
        switch (command)
        {
        case NOTIFY_SLEEP_NEAR_TIMEOUT:
            notifySleepNearTimeoutReceived = true;
            break;
        case NOTIFY_SLEEP_TIMEOUT:
            notifySleepTimeoutReceived = true;
            break;
        }
    }
};

int main(int argc, char **argv)
{
    // setup logging
    setup_logging();
    log4cxx::LoggerPtr narratorLogger(log4cxx::Logger::getLogger("kolibre.narrator"));
    narratorLogger->setLevel(log4cxx::Level::getFatal());
    log4cxx::LoggerPtr playerLogger(log4cxx::Logger::getLogger("kolibre.player"));
    playerLogger->setLevel(log4cxx::Level::getFatal());
    log4cxx::LoggerPtr doLogger(log4cxx::Logger::getLogger("kolibre.daisyonline"));
    doLogger->setLevel(log4cxx::Level::getFatal());
    log4cxx::LoggerPtr onlineLogger(log4cxx::Logger::getLogger("kolibre.clientcore.daisyonlinenode"));
    onlineLogger->setLevel(log4cxx::Level::getFatal());
    log4cxx::LoggerPtr naviLogger(log4cxx::Logger::getLogger("kolibre.clientcore.navi"));
    naviLogger->setLevel(log4cxx::Level::getFatal());

    // setup handlers
    Handle_NotifyCommands notifyHandler;
    notifyHandler.listen();

    ClientCore clientcore;
    clientcore.start();

    // sleep timer off by default
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() == -1);
    assert(clientcore.getSleepTimerSetting() == -1);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_OFF);

    // activate sleep timer
    cout << "activating sleep timer manually" << endl;
    clientcore.setSleepTimerTimeLeft(10);
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() > 10 * 60 - 5);
    assert(clientcore.getSleepTimerSetting() == 10);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_ON);

    // deactivate sleep timer
    cout << "deactivating sleep timer manually" << endl;
    clientcore.setSleepTimerTimeLeft(-1);
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() == -1);
    assert(clientcore.getSleepTimerSetting() == -1);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_OFF);

    /*
     * TODO: investigate why sleep timer not works with commands
     *
    // activate sleep timer with commands
    cout << "activating sleep timer with command SLEEP_15" << endl;
    clientcore.pushCommand(ClientCore::SLEEP_15);
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() > 15 * 60 - 5);
    assert(clientcore.getSleepTimerSetting() == 15);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_ON);

    cout << "activating sleep timer with command SLEEP_30" << endl;
    clientcore.pushCommand(ClientCore::SLEEP_30);
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() > 30 * 60 - 5);
    assert(clientcore.getSleepTimerSetting() == 30);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_ON);

    cout << "activateing sleep timer with command SLEEP_60" << endl;
    clientcore.pushCommand(ClientCore::SLEEP_60);
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() > 60 * 60 - 5);
    assert(clientcore.getSleepTimerSetting() == 60);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_ON);

    cout << "deactivating sleep timer with command SLEEP_OFF" << endl;
    clientcore.pushCommand(ClientCore::SLEEP_OFF);
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() == -1);
    assert(clientcore.getSleepTimerSetting() == -1);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_OFF);
    */

    // activate sleep timer
    cout << "activating sleep timer manually" << endl;
    clientcore.setSleepTimerTimeLeft(1);
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() > 60 - 5);
    assert(clientcore.getSleepTimerSetting() == 1);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_ON);

    // wait for near timeout notfication
    int maxWait = 31;
    notifySleepNearTimeoutReceived = false;
    while (notifySleepNearTimeoutReceived == false)
    {
        cout << "waiting for 'near timeout' notification, " << maxWait << " second(s) left" << endl;
        sleep(1);
        maxWait--;
        if (maxWait == 0) break;;
    }

    // we should now have received a near timeout notification
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() < 30);
    assert(clientcore.getSleepTimerSetting() == 1);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_NEAR_TIMEOUT);
    assert(clientcore.isRunning() == true);

    // deactivate sleep timer by sending any push command
    clientcore.pushCommand(ClientCore::SLEEP_OFF);
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() == -1);
    assert(clientcore.getSleepTimerSetting() == -1);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_OFF);

    // wait for deactivate action
    sleep(5);

    // activate sleep timer
    cout << "activating sleep timer manually" << endl;
    clientcore.setSleepTimerTimeLeft(1);
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() > 60 - 5);
    assert(clientcore.getSleepTimerSetting() == 1);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_ON);

    // wait for near timeout notfication
    maxWait = 31;
    notifySleepNearTimeoutReceived = false;
    while (notifySleepNearTimeoutReceived == false)
    {
        cout << "waiting for 'near timeout' notification, " << maxWait << " second(s) left" << endl;
        sleep(1);
        maxWait--;
        if (maxWait == 0) break;;
    }

    // we should now have received a near timeout notification
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() < 30);
    assert(clientcore.getSleepTimerSetting() == 1);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_NEAR_TIMEOUT);
    assert(clientcore.isRunning() == true);

    // wait for timeout notfication
    maxWait = 31;
    while (notifySleepTimeoutReceived == false)
    {
        cout << "waiting for 'timeout' notification, " << maxWait << " second(s) left" << endl;
        sleep(1);
        maxWait--;
        if (maxWait == 0) break;;
    }

    // we should now have received a timeout notification
    printSleepTimerInfo(clientcore);
    assert(clientcore.getSleepTimerTimeLeft() < 0);
    assert(clientcore.getSleepTimerSetting() == 1);
    assert(clientcore.getSleepTimerState() == SLEEP_TIMER_TIMED_OUT);
    assert(clientcore.isRunning() == false);

    return 0;
}
