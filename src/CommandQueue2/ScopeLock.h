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

#ifndef SCOPELOCK_H
#define SCOPELOCK_H

#include <pthread.h>

/**
 * @class ScopeLock
 *
 * Convenience class for keeping a mutex locked in a scope and
 * unlocking it automatically when going out of scope.
 */

class ScopeLock
{
public:
    /**
     * Lock the mutex @param mutex
     */
    ScopeLock(pthread_mutex_t& mutex) : mutex_(&mutex)
    {
        pthread_mutex_lock(mutex_);
    }

    /**
     * Unlock the mutex
     */
    ~ScopeLock()
    {
        pthread_mutex_unlock(mutex_);
    }
private:
    pthread_mutex_t* mutex_;
};

#endif
