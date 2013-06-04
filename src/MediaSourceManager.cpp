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

#include "MediaSourceManager.h"

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr mediaSourceManLog(log4cxx::Logger::getLogger("kolibre.clientcore.mediasourcemanager"));

using namespace std;

MediaSourceManager * MediaSourceManager::pinstance = 0;

MediaSourceManager * MediaSourceManager::Instance()
{
    if (pinstance == 0)
    {
        pinstance = new MediaSourceManager;
    }

    return pinstance;
}

void MediaSourceManager::DeleteInstance()
{
    pthread_mutex_lock(&manager_mutex);
    // nothing to do
    pthread_mutex_unlock(&manager_mutex);
    pthread_mutex_destroy(&manager_mutex);
    MediaSourceManager::pinstance=NULL;
    delete this;
}

MediaSourceManager::MediaSourceManager()
{
    LOG4CXX_TRACE(mediaSourceManLog, "Constructor");

    pthread_mutex_init (&manager_mutex, NULL);
    LOG4CXX_TRACE(mediaSourceManLog, "MediaSourceManager mutex initialized");
}

MediaSourceManager::~MediaSourceManager()
{
    LOG4CXX_INFO(mediaSourceManLog, "Deleting MediaSourceManager instance");
}
