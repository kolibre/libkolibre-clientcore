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

#ifndef _MEDIASOURCEMANAGER_H
#define _MEDIASOURCEMANAGER_H

#include <string>
#include <vector>
#include <pthread.h>

class MediaSourceManager
{
public:
    static MediaSourceManager *Instance();
    void DeleteInstance();

    // functions for manipulating DaisyOnline services
    int addDaisyOnlineService(const std::string name, const std::string url, const std::string username, const std::string password, const bool remember = false);
    int getDaisyOnlineServices();
    int getDaisyOnlineServiceIndex(const std::string name);
    bool removeDaisyOnlineService(const std::string name);
    bool removeDaisyOnlineService(int index);
    bool clearDaisyOnlineServies();
    bool setDOSname(int index, const std::string name);
    std::string getDOSname(int index);
    bool setDOSurl(int index, const std::string url);
    std::string getDOSurl(int index);
    bool setDOSusername(int index, const std::string username);
    std::string getDOSusername(int index);
    bool setDOSpassword(int index, const std::string password);
    std::string getDOSpassword(int index);
    bool setDOSremember(int index, bool remember);
    bool getDOSremember(int index);

    // functions for manipulating file systems paths
    int addFileSystemPath(const std::string name, const std::string path);
    int getFileSystemPaths();
    int getFileSystemPathIndex(const std::string);
    bool removeFileSystemPath(const std::string name);
    bool removeFileSystemPath(int index);
    bool clearFileSystemsPaths();
    bool setFSPname(int index, const std::string name);
    std::string getFSPname(int index);
    bool setFSPpath(int index, const std::string path);
    std::string getFSPpath(int index);

protected:
    MediaSourceManager();

private:
    pthread_mutex_t manager_mutex;
    static MediaSourceManager *pinstance;
    ~MediaSourceManager();
    bool DOSindexOutOfRange(int index);
    bool FSPindexOutOfRange(int index);

    /**
     * A data type to hold information about a DaisyOnline service
     */
    struct DaisyOnlineService
    {
        DaisyOnlineService(const std::string name, const std::string url, const std::string username, const std::string password, bool remember) :
            name(name), url(url), username(username), password(password), remember(remember)
        {
        }

        /**
         *  The name of the DaisyOnline service to distinguish it from other services
         */
        std::string name;

        /**
         * URL for the service
         */
        std::string url;

        /**
         * Username to be used for authenticating a user on the service
         */
        std::string username;

        /**
         * Password to be used for authenticating a user on the service
         */
        std::string password;

        /**
         * Boolean to determine if we shall remember username and password
         */
        bool remember;
    };

    /**
     * A data type to hold information about a file system path
     */
    struct FileSystemPath
    {
        FileSystemPath(const std::string name, const std::string path) :
            name(name), path(path)
        {
        }

        /**
         * The name of the path to distinguish it from other paths
         */
        std::string name;

        /**
         * The path on the file system
         */
        std::string path;
    };

    std::vector<DaisyOnlineService> DaisyOnlineServices;
    std::vector<FileSystemPath> FileSystemPaths;
};

#endif // _MEDIASOURCEMANAGER_H
