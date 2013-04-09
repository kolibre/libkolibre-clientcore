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

#include <iostream>
#include <assert.h>
#include <stdlib.h>

#include "CommandQueue2/CommandQueue.h"

using namespace std;

void handle_int(int data)
{
    cout << __PRETTY_FUNCTION__ << " " << data << endl;
}

void handle_string(string data)
{
    cout << __PRETTY_FUNCTION__ << " " << data << endl;
}

int main()
{
    // one handler for each command type
    cq2::Handler<int> handler1(&handle_int);
    handler1.listen();
    cq2::Handler<string> handler2(&handle_string);
    handler2.listen();

    // push two commands to the queue
    cq2::Command<int> digit(1);
    digit();
    cq2::Command<string> word("word");
    word();

    // we should be able to dispatch two commands
    assert(cq2::Dispatcher::instance().dispatchCommand());
    assert(cq2::Dispatcher::instance().dispatchCommand());

    // push two more commands to the queue
    digit();
    word();

    // after the queue has benn flashed we should no be able to dispatch any commands
    cq2::Dispatcher::instance().flushQueue();
    assert(not cq2::Dispatcher::instance().dispatchCommand());
    assert(not cq2::Dispatcher::instance().dispatchCommand());

    return 0;
}
