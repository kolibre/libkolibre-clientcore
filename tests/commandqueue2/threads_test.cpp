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
#include <list>
#include <assert.h>

#include "CommandQueue2/CommandQueue.h"

using namespace std;

// TEST

static bool running = true;

#include <stdlib.h>

void* trad1(void* count)
{
    long* counter = (long*) count;

    while (running)
    {
        cq2::Command<int> evI(10);
        evI();
        (*counter)++;
        usleep(900 + (rand() % 10));
    }

    pthread_exit(NULL);
}

void* trad2(void* count)
{
    long* counter = (long*) count;

    while (running)
    {
        cq2::Command<string> evS("text");
        evS();
        (*counter)++;
        usleep(1000 + (rand() % 10));
    }

    pthread_exit(NULL);
}

void signal_handler(int)
{
    running = false;
}

#include <signal.h>
#include <pthread.h>

static long totalCount = 0;
static long innerEmitCount = 0;

void handle_int(int data)
{
    totalCount++;
    cout << __PRETTY_FUNCTION__ << data << endl;

    cq2::Handler<int> add_handler_from_within_handler(&handle_int);
    add_handler_from_within_handler.listen();

    if (running and totalCount % 7 == 0)
    {
        cq2::Command<string> inner(" string signal emitted from within int handler");
        inner();
        innerEmitCount++;
    }
}

void handle_string(string data)
{
    totalCount++;
    cout << __PRETTY_FUNCTION__ << data << endl;

    cq2::Handler<int> add_handler_from_within_handler(&handle_int);
    add_handler_from_within_handler.listen();

    if (running and totalCount % 8 == 0)
    {
        cq2::Command<string> inner(" string signal emitted from within string handler");
        inner();
        innerEmitCount++;
    }

}

int main()
{
    // One handler for each command type
    cq2::Handler<int> handler1(&handle_int);
    handler1.listen();

    cq2::Handler<string> h;
    {
        cq2::Handler<string> handler2(&handle_string);
        handler2.listen();
        h = handler2;
        h.listen();
    }

    pthread_t thread0;
    long count0 = 0;
    pthread_create(&thread0, NULL, trad1, &count0);

    pthread_t thread1;
    long count1 = 0;
    pthread_create(&thread1, NULL, trad1, &count1);

    pthread_t thread2;
    long count2 = 0;
    pthread_create(&thread2, NULL, trad2, &count2);

    pthread_t thread3;
    long count3 = 0;
    pthread_create(&thread3, NULL, trad2, &count3);

    struct sigaction act;
    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (0 != sigaction(SIGINT, &act, NULL))
    {
        return 1;
    }
    if (0 != sigaction(SIGALRM, &act, NULL))
    {
        return 1;
    }

    // Stop after 10s
    alarm(10);

    // Handle the pending commands in order

    bool emptyQueue = true;
    while (running or not emptyQueue)
    {
        emptyQueue = not cq2::Dispatcher::instance().dispatchCommand();
        if (emptyQueue)
        {
            cout << "empty queue, press Ctrl+C or wait 10s" << endl;
            usleep(10 + (rand() % 900));
        }
    }

    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    while (cq2::Dispatcher::instance().dispatchCommand())
    {
        cout << "Emptying the queue" << endl;
    }

    if (totalCount != (count0 + count1 + count2 + count3 + innerEmitCount))
    {
        cout << "total: " << totalCount << endl;
        cout << "sum: " << (count0 + count1 + count2 + count3) << endl;
        assert(false);
    }

    cout << "The end, alles OK!" << endl;
}
