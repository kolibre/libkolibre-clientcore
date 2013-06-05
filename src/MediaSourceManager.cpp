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

int MediaSourceManager::addDaisyOnlineService(const std::string name, const std::string url, const std::string username, const std::string password, const bool remember)
{
    if (name.empty())
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot add DaisyOnlineService without name");
        return -1;
    }
    else if (url.empty())
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot add DaisyOnlineService without url");
        return -1;
    }

    pthread_mutex_lock(&manager_mutex);
    std::vector<DaisyOnlineService>::iterator it;
    for (it = DaisyOnlineServices.begin(); it != DaisyOnlineServices.end(); ++it)
    {
        if (name == it->name)
        {
            LOG4CXX_WARN(mediaSourceManLog, "DaisyOnlineService with name '" << name << "' has already been added");
            pthread_mutex_unlock(&manager_mutex);
            return -1;
        }
    }

    DaisyOnlineService service(name, url, username, password, remember);
    DaisyOnlineServices.push_back(service);
    int index = DaisyOnlineServices.size()-1;
    pthread_mutex_unlock(&manager_mutex);

    return index;
}

int MediaSourceManager::getDaisyOnlineServices()
{
    return DaisyOnlineServices.size();
}

int MediaSourceManager::getDaisyOnlineServiceIndex(const std::string name)
{
    int index = -1;
    pthread_mutex_lock(&manager_mutex);
    std::vector<DaisyOnlineService>::iterator it;
    for (it = DaisyOnlineServices.begin(); it != DaisyOnlineServices.end(); ++it)
    {
        if (name == it->name)
        {
            index = it - DaisyOnlineServices.begin();
            LOG4CXX_DEBUG(mediaSourceManLog, "DaisyOnlineService with name '" <<  name << "' was found at index " << index);
            break;
        }
    }
    pthread_mutex_unlock(&manager_mutex);

    if (index == -1)
    {
        LOG4CXX_WARN(mediaSourceManLog, "DaisyOnlineService with name '" <<  name << "' not found");
    }
    return index;
}

bool MediaSourceManager::removeDaisyOnlineService(const std::string name)
{
    bool removed = false;
    pthread_mutex_lock(&manager_mutex);
    std::vector<DaisyOnlineService>::iterator it;
    for (it = DaisyOnlineServices.begin(); it != DaisyOnlineServices.end(); ++it)
    {
        if (name == it->name)
        {
            int index = it - DaisyOnlineServices.begin();
            LOG4CXX_DEBUG(mediaSourceManLog, "Removing DaisyOnlineService with name '" << name << "'at index " << index);
            DaisyOnlineServices.erase(DaisyOnlineServices.begin() + index);
            removed = true;
            break;
        }
    }
    pthread_mutex_unlock(&manager_mutex);
    return removed;
}

bool MediaSourceManager::removeDaisyOnlineService(int index)
{
    bool removed = false;
    pthread_mutex_lock(&manager_mutex);
    if (DOSindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot remove DaisyOnlineService, index out of range");
    }
    else
    {
        LOG4CXX_DEBUG(mediaSourceManLog, "Removing DaisyOnlineService at index " << index);
        DaisyOnlineServices.erase(DaisyOnlineServices.begin() + index);
        removed = true;
    }
    pthread_mutex_unlock(&manager_mutex);
    return removed;
}

bool MediaSourceManager::clearDaisyOnlineServies()
{
    LOG4CXX_DEBUG(mediaSourceManLog, "Deleting all added DaisyOnlineServices");
    DaisyOnlineServices.clear();
    return DaisyOnlineServices.size() == 0 ? true : false;
}

bool MediaSourceManager::setDOSname(int index, const std::string name)
{
    if (name.empty())
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set empty name for DaisyOnlineService");
        return false;
    }
    else if (DOSindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set name for DaisyOnlineService, index out of range");
        return false;
    }

    pthread_mutex_lock(&manager_mutex);
    std::vector<DaisyOnlineService>::iterator it;
    for (it = DaisyOnlineServices.begin(); it != DaisyOnlineServices.end(); ++it)
    {
        if (name == it->name)
        {
            LOG4CXX_WARN(mediaSourceManLog, "Cannot set name for DaisyOnlineService, name already exist");
            pthread_mutex_unlock(&manager_mutex);
            return false;
        }
    }
    LOG4CXX_DEBUG(mediaSourceManLog, "Changing name for DaisyOnlineService to '" << name << "' at index " << index);
    DaisyOnlineServices[index].name = name;
    pthread_mutex_unlock(&manager_mutex);
    return true;
}

std::string MediaSourceManager::getDOSname(int index)
{
    std::string name = "";
    pthread_mutex_lock(&manager_mutex);
    if (DOSindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot get name for DaisyOnlineService, index out of range");
    }
    else
    {
        name = DaisyOnlineServices[index].name;
    }
    pthread_mutex_unlock(&manager_mutex);
    return name;
}

bool MediaSourceManager::setDOSurl(int index, const std::string url)
{
    if (url.empty())
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set empty url for DaisyOnlineService");
        return false;
    }
    else if (DOSindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set url for DaisyOnlineService, index out of range");
        return false;
    }

    pthread_mutex_lock(&manager_mutex);
    LOG4CXX_DEBUG(mediaSourceManLog, "Changing url for DaisyOnlineService to '" << url << "' at index " << index);
    DaisyOnlineServices[index].url = url;
    pthread_mutex_unlock(&manager_mutex);
    return true;
}

std::string MediaSourceManager::getDOSurl(int index)
{
    std::string url = "";
    pthread_mutex_lock(&manager_mutex);
    if (DOSindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot get url for DaisyOnlineService, index out of range");
    }
    else
    {
        url = DaisyOnlineServices[index].url;
    }
    pthread_mutex_unlock(&manager_mutex);
    return url;
}

bool MediaSourceManager::setDOSusername(int index, const std::string username)
{
    if (username.empty())
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set empty username for DaisyOnlineService");
        return false;
    }
    else if (DOSindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set username for DaisyOnlineService, index out of range");
        return false;
    }

    pthread_mutex_lock(&manager_mutex);
    LOG4CXX_DEBUG(mediaSourceManLog, "Changing username for DaisyOnlineService at index " << index);
    DaisyOnlineServices[index].username = username;
    pthread_mutex_unlock(&manager_mutex);
    return true;
}

std::string MediaSourceManager::getDOSusername(int index)
{
    std::string username;
    pthread_mutex_lock(&manager_mutex);
    if (DOSindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot get username for DaisyOnlineService, index out of range");
    }
    else
    {
        username = DaisyOnlineServices[index].username;
    }
    pthread_mutex_unlock(&manager_mutex);
    return username;
}

bool MediaSourceManager::setDOSpassword(int index, const std::string password)
{
    if (password.empty())
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set empty password for DaisyOnlineService");
        return false;
    }
    else if (DOSindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set password for DaisyOnlineService, index out of range");
        return false;
    }

    pthread_mutex_lock(&manager_mutex);
    LOG4CXX_DEBUG(mediaSourceManLog, "Changing password for DaisyOnlineService at index " << index);
    DaisyOnlineServices[index].password = password;
    pthread_mutex_unlock(&manager_mutex);
    return true;
}

std::string MediaSourceManager::getDOSpassword(int index)
{
    std::string password;
    pthread_mutex_lock(&manager_mutex);
    if (DOSindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot get password for DaisyOnlineService, index out of range");
    }
    else
    {
        password = DaisyOnlineServices[index].password;
    }
    pthread_mutex_unlock(&manager_mutex);
    return password;
}

bool MediaSourceManager::setDOSremember(int index, bool remember)
{
    if (DOSindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set remember for DaisyOnlineService, index out of range");
        return false;
    }

    pthread_mutex_lock(&manager_mutex);
    LOG4CXX_DEBUG(mediaSourceManLog, "Changing remember for DaisyOnlineService at index " << index);
    DaisyOnlineServices[index].remember = remember;
    pthread_mutex_unlock(&manager_mutex);
    return true;
}

bool MediaSourceManager::getDOSremember(int index)
{
    bool remember;
    pthread_mutex_lock(&manager_mutex);
    if (DOSindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot get remember for DaisyOnlineService, index out of range");
    }
    else
    {
        remember = DaisyOnlineServices[index].remember;
    }
    pthread_mutex_unlock(&manager_mutex);
    return remember;
}

int MediaSourceManager::addFileSystemPath(const std::string name, const std::string path)
{
    if (name.empty())
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot add FileSystemPath without name");
        return -1;
    }
    else if (path.empty())
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot add FileSystemPath without path");
        return -1;
    }

    pthread_mutex_lock(&manager_mutex);
    std::vector<FileSystemPath>::iterator it;
    for (it = FileSystemPaths.begin(); it != FileSystemPaths.end(); ++it)
    {
        if (name == it->name)
        {
            LOG4CXX_WARN(mediaSourceManLog, "File system path with name '" << name << "' has already been added");
            pthread_mutex_unlock(&manager_mutex);
            return -1;
        }
    }

    FileSystemPath filepath(name, path);
    FileSystemPaths.push_back(filepath);
    int index = FileSystemPaths.size()-1;
    pthread_mutex_unlock(&manager_mutex);

    return index;
}

int MediaSourceManager::getFileSystemPaths()
{
    return FileSystemPaths.size();
}

int MediaSourceManager::getFileSystemPathIndex(const std::string name)
{
    int index = -1;
    pthread_mutex_lock(&manager_mutex);
    std::vector<FileSystemPath>::iterator it;
    for (it = FileSystemPaths.begin(); it != FileSystemPaths.end(); ++it)
    {
        if (name == it->name)
        {
            index = it - FileSystemPaths.begin();
            LOG4CXX_DEBUG(mediaSourceManLog, "File system path with name '" <<  name << "' was found at index " << index);
            break;
        }
    }
    pthread_mutex_unlock(&manager_mutex);

    if (index == -1)
    {
        LOG4CXX_WARN(mediaSourceManLog, "File system path with name '" <<  name << "' not found");
    }

    return index;
}

bool MediaSourceManager::removeFileSystemPath(const std::string name)
{
    bool removed = false;
    pthread_mutex_lock(&manager_mutex);
    std::vector<FileSystemPath>::iterator it;
    for (it = FileSystemPaths.begin(); it != FileSystemPaths.end(); ++it)
    {
        if (name == it->name)
        {
            int index = it - FileSystemPaths.begin();
            LOG4CXX_DEBUG(mediaSourceManLog, "Removing FileSystemPath with name '" << name << "'at index " << index);
            FileSystemPaths.erase(FileSystemPaths.begin() + index);
            removed = true;
            break;
        }
    }
    pthread_mutex_unlock(&manager_mutex);
    return removed;
}

bool MediaSourceManager::removeFileSystemPath(int index)
{
    bool removed = false;
    pthread_mutex_lock(&manager_mutex);
    if (FSPindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot remove FileSystemPath, index out of range");
    }
    else
    {
        LOG4CXX_DEBUG(mediaSourceManLog, "Removing FileSystemPath at index " << index);
        FileSystemPaths.erase(FileSystemPaths.begin() + index);
        removed = true;
    }
    pthread_mutex_unlock(&manager_mutex);
    return removed;
}

bool MediaSourceManager::clearFileSystemsPaths()
{
    LOG4CXX_DEBUG(mediaSourceManLog, "Deleting all added FileSystemPaths");
    FileSystemPaths.clear();
    return FileSystemPaths.size() == 0 ? true : false;
}

bool MediaSourceManager::setFSPname(int index, const std::string name)
{
    if (name.empty())
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set empty name for FileSystemPath");
        return false;
    }
    else if (FSPindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set name for FileSystemPath, index out of range");
        return false;
    }

    pthread_mutex_lock(&manager_mutex);
    std::vector<FileSystemPath>::iterator it;
    for (it = FileSystemPaths.begin(); it != FileSystemPaths.end(); ++it)
    {
        if (name == it->name)
        {
            LOG4CXX_WARN(mediaSourceManLog, "Cannot set name for FileSystemPath, name already exist");
            pthread_mutex_unlock(&manager_mutex);
            return false;
        }
    }
    LOG4CXX_DEBUG(mediaSourceManLog, "Changing name for FileSystemPath to '" << name << "' at index " << index);
    FileSystemPaths[index].name = name;
    pthread_mutex_unlock(&manager_mutex);
    return true;
}

std::string MediaSourceManager::getFSPname(int index)
{
    std::string name = "";
    pthread_mutex_lock(&manager_mutex);
    if (FSPindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot get name for FileSystemPath, index out of range");
    }
    else
    {
        name = FileSystemPaths[index].name;
    }
    pthread_mutex_unlock(&manager_mutex);
    return name;
}

bool MediaSourceManager::setFSPpath(int index, const std::string path)
{
    if (path.empty())
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set empty path for FileSystemPath");
        return false;
    }
    else if (FSPindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot set path for FileSystemPath, index out of range");
        return false;
    }

    pthread_mutex_lock(&manager_mutex);
    LOG4CXX_DEBUG(mediaSourceManLog, "Changing path for FileSystemPath to '" << path << "' at index " << index);
    FileSystemPaths[index].path = path;
    pthread_mutex_unlock(&manager_mutex);
    return true;
}

std::string MediaSourceManager::getFSPpath(int index)
{
    std::string path = "";
    pthread_mutex_lock(&manager_mutex);
    if (FSPindexOutOfRange(index))
    {
        LOG4CXX_WARN(mediaSourceManLog, "Cannot get path for FileSystemPath, index out of range");
    }
    else
    {
        path = FileSystemPaths[index].path;
    }
    pthread_mutex_unlock(&manager_mutex);
    return path;
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

bool MediaSourceManager::DOSindexOutOfRange(int index)
{
    if (DaisyOnlineServices.size() == 0)
    {
        return true;
    }
    else if (index < 0 || index > DaisyOnlineServices.size()-1)
    {
        return true;
    }
    return false;
}

bool MediaSourceManager::FSPindexOutOfRange(int index)
{
    if (FileSystemPaths.size() == 0)
    {
        return true;
    }
    else if (index < 0 || index > FileSystemPaths.size()-1)
    {
        return true;
    }
    return false;
}
