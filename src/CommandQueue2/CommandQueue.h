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

#ifndef COMMANDQUEUE2_COMMANDQUEUE_H
#define COMMANDQUEUE2_COMMANDQUEUE_H

#include <list>
#include "ScopeLock.h"

#include <boost/function.hpp>

#include "assert.h"

/**
 * @class cq2::Command
 * type-safe container. Used to create and send commands.
 *
 * @class cq2::Dispatcher
 * Singleton, handling ordered dispatching of commands to matching
 * Handler instances.
 *
 * @class cq2::Handler
 * used in the receiving end. Can be used as a base class by
 * overriding void handle(DataT data) or directly by passing a
 * handler function to the constructor.
 *
 * @class cq2::CommandQueue
 * used internally to contain Commands in a type-safe manner.
 * See cq2::Command and cq2::Handler for information on how to send and receive commands.
 *
 * @class cq2::IQueue
 * used internally by Dispatcher to keep track of dispatching queues.
 *
 * @class cq2::ICommand
 * used internally by Dispatcher to keep track of dipatching order.
 */

namespace cq2
{

class ICommand;

class IQueue
{
private:
    virtual void emit(ICommand*) = 0;
    friend class Dispatcher;
};

class ICommand
{
public:
    IQueue* queue; /**< Used by Dispatcher to find the containing CommandQueue */
    ICommand* nextCommand; /**< Used by Dispatcher to keep track of dispatch order */

    /**
     * Constructor
     */
    ICommand() : nextCommand(0) {}
};

template<typename DataT>
class Command: public ICommand
{
public:
    DataT data; /**< Contained data */

    /**
     * Command constructor, takes data to be contained as parameter
     */
    Command(DataT d = DataT()) : data(d) {}

    /**
     * Send the Command. Pushes this Command onto the CommandQueue<DataT>.
     */
    void operator()();
};

class Dispatcher
{
public:
    /**
     * Singleton instance
     */
    static Dispatcher& instance()
    {
        static Dispatcher inst;
        return inst;
    }

    /**
     * Dispatch one command in this thread context.
     *
     * @returns false if no command was dispatched (empty queue)
     * @returns true if one command was dispatched
     */
    bool dispatchCommand()
    {
        ICommand* toHandle = 0;

        {
            ScopeLock lock(dispatcherMutex);

            if (first == NULL)
                return false;

            toHandle = first;
            first = toHandle->nextCommand;
        }

        toHandle->queue->emit(toHandle);

        return true;
    }

    /**
     * Flush all commands in the queue.
     */
    void flushQueue()
    {
        ScopeLock lock(dispatcherMutex);
        if (first != NULL)
            first = NULL;
    }
private:
    Dispatcher()
    {
        pthread_mutex_init(&dispatcherMutex, NULL);
    }

    // Locked in CommandQueue<DataT>
    void queueCommand(ICommand* ev)
    {
        if (first == NULL)
        {
            first = ev;
            last = ev;
            return;
        }

        last->nextCommand = ev;
        last = ev;
    }

private:
    // Used as a global lock for simplicity. Avoids live-lock scenarios.
    pthread_mutex_t dispatcherMutex;

    ICommand* first;
    ICommand* last;

    template<typename DataT>
    friend class CommandQueue;
};

template<typename DataT>
class Handler
{
public:
    /**
     * Constructor used to register external handler
     */
    Handler(boost::function<void(DataT)> handler) : handler_p(handler) {}

    /**
     * Constructor used by subclasses
     */
    Handler() {}

    /**
     * Override if inheritance is used.
     */
    virtual void handle(DataT data)
    {
        if (not handler_p.empty())
            (handler_p)(data);
    }

    /**
     * Start receiving Commands.
     */
    void listen();

    /**
     * Stop receiving Commands.
     */
    void ignore();

    /**
     * Upon destruction the Handler automatically removes itself from the CommandQueue.
     */
    virtual ~Handler();

private:
    boost::function<void(DataT)> handler_p;
};

template<typename DataT>
class CommandQueue: public IQueue
{
    std::list<Handler<DataT>*> handlers;
    std::list<Command<DataT> > commands;
public:
    /**
     * Singleton instance
     */
    static CommandQueue& instance()
    {
        static CommandQueue<DataT> inst;
        return inst;
    }

    // Used by Handler
    void addHandler(Handler<DataT>* handler)
    {
        ScopeLock lock(Dispatcher::instance().dispatcherMutex);
        handlers.push_back(handler);
    }

    // Used by Handler
    void removeHandler(Handler<DataT>* handler)
    {
        ScopeLock lock(Dispatcher::instance().dispatcherMutex);
        handlers.remove(handler);
    }

    // Used by Command
    void enqueue(Command<DataT> command)
    {
        command.queue = this;

        {
            ScopeLock lock(Dispatcher::instance().dispatcherMutex);
            commands.push_back(command);
            Dispatcher::instance().queueCommand(&commands.back());
        }

    }

private:

    // Used by Dispatcher to dispatch command
    void emit(ICommand* command)
    {
        std::list<Handler<DataT>*> tmp_handlers;
        DataT tmp_eventData;

        { // LOCK and make a copy of the handlers
            ScopeLock lock(Dispatcher::instance().dispatcherMutex);

            assert( &commands.front() == command);
            // make copies
            tmp_handlers = handlers;
            tmp_eventData = commands.front().data;

            commands.pop_front();
        } // UNLOCK

        typename std::list<Handler<DataT>*>::iterator handler_it = tmp_handlers.begin();

        while (handler_it != tmp_handlers.end())
        {
            (*handler_it)->handle(tmp_eventData);
            handler_it++;
        }
    }
};

// Member functions, use of CommandQueue<> requires them to be after CommandQueue

template<typename DataT>
void Command<DataT>::operator()()
{
    CommandQueue<DataT>::instance().enqueue(*this);
}

template<typename DataT>
void Handler<DataT>::listen()
{
    CommandQueue<DataT>::instance().addHandler(this);
}

template<typename DataT>
void Handler<DataT>::ignore()
{
    CommandQueue<DataT>::instance().removeHandler(this);
}

template<typename DataT>
Handler<DataT>::~Handler()
{
    CommandQueue<DataT>::instance().removeHandler(this);
}

}

#endif
