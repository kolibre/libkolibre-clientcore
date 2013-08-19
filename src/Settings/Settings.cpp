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

#include "Db.h"
#include "Settings.h"

#include <string>
#include <iostream>
#include <fstream>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr settingsLog(log4cxx::Logger::getLogger("kolibre.clientcore.settings"));

using namespace std;

#define DB_VERSION 1

bool filecopy(string fromfile, string tofile)
{
    LOG4CXX_DEBUG(settingsLog, "copy(" << fromfile << ", "<< tofile << ")");

    fstream infile(fromfile.c_str(), fstream::in | fstream::binary);
    fstream outfile(tofile.c_str(), fstream::out | fstream::trunc | fstream::binary);
    if (not (outfile and infile))
    {
        return false;
    }
    outfile << infile.rdbuf();

    return true;
}

Settings * Settings::pinstance = 0;

Settings * Settings::Instance()
{
    if (pinstance == 0)
    {
        pinstance = new Settings;
    }

    return pinstance;
}

void Settings::DeleteInstance()
{
    pthread_mutex_lock(&settings_mutex);
    //Nothing going on
    pthread_mutex_unlock(&settings_mutex);
    pthread_mutex_destroy(&settings_mutex);
    Settings::pinstance=NULL;
    delete this;
}

Settings::Settings():
    mCurrentDomain("localhost")
{
    LOG4CXX_TRACE(settingsLog, "Constructor");

    pthread_mutex_init (&settings_mutex, NULL);
    LOG4CXX_TRACE(settingsLog, "Settings mutex initialized");

    string settingsfile = Utils::getDatapath() + "settings.db";

    LOG4CXX_INFO(settingsLog, "Opening settings.db in '"<< settingsfile << "'");

    bool settingsFileExists = fstream(settingsfile.c_str(), fstream::in).is_open();

    pDBHandle = new settings::DB(settingsfile);

    if (!pDBHandle->connect())
    {
        LOG4CXX_ERROR(settingsLog, "Could not open settings.db: '" << settingsfile << "', error '" << pDBHandle->getLasterror() << "'");

        delete pDBHandle;
        pDBHandle = NULL;
        return;
    }

    if (settingsFileExists)
    {
        const int fileVersion = getVersion();

        if (fileVersion != DB_VERSION)
        {
            LOG4CXX_DEBUG(settingsLog, "DB_VERSION missmatch, migrating db");
            stringstream backupName;
            backupName << settingsfile << fileVersion;

            delete pDBHandle;
            pDBHandle = NULL;

            if (not filecopy(settingsfile, backupName.str()))
            {
                LOG4CXX_ERROR(settingsLog, "Could not make backup of old settings.db");
                return;
            }

            if (fileVersion > DB_VERSION)
            {
                //Reopen a backup or an empty file
                if (not restoreClosestDbBackup(settingsfile))
                {
                    settingsFileExists = false; // Trigger a setup downstairs
                    fstream f(settingsfile.c_str(), fstream::out | fstream::trunc); //empty the file
                }
            }

            pDBHandle = new settings::DB(settingsfile);

            if (!pDBHandle->connect())
            {
                LOG4CXX_ERROR(settingsLog, "Could not reopen settings.db: '" << settingsfile << "', error '" << pDBHandle->getLasterror() << "'");
                delete pDBHandle;
                pDBHandle = NULL;
                return;
            }

            if (settingsFileExists && getVersion() < DB_VERSION)
            {
                // DO migrate up to DB_VERSION here, 0->1 requires none?
                if (not setVersion(DB_VERSION))
                {
                    LOG4CXX_ERROR(settingsLog, "migration failed");
                }
            }
        }
    }

    if (not settingsFileExists)
    //Verify database structure
    {
        LOG4CXX_DEBUG(settingsLog, "Verifying database structure");

        if (!pDBHandle->prepare("create table if not exists setting (setting TEXT, value TEXT, type INT, domain TEXT, UNIQUE(setting, domain))"))
        {
            LOG4CXX_ERROR(settingsLog, "Could not create/open settings table: '" << pDBHandle->getLasterror() << "'");
            return;
        }

        if (!pDBHandle->perform())
        {
            LOG4CXX_ERROR(settingsLog, "Create table setting failed '" << pDBHandle->getLasterror() << "'");
            return;
        }

        if (!pDBHandle->prepare("create table if not exists parameter (setting_id INT, key TEXT, value TEXT, type INT, UNIQUE(setting_id, key))"))
        {
            LOG4CXX_ERROR(settingsLog, "Could not create/open parameters table: '" << pDBHandle->getLasterror() << "'");
            return;
        }

        if (!pDBHandle->perform())
        {
            LOG4CXX_ERROR(settingsLog, "Create table parameter failed '" << pDBHandle->getLasterror() << "'");
            return;
        }

        if (not setVersion(DB_VERSION))
        {
            LOG4CXX_ERROR(settingsLog, "Setting db version failed");
        }
        LOG4CXX_DEBUG(settingsLog, "Database structure verified");
    }

}

Settings::~Settings()
{
    LOG4CXX_INFO(settingsLog, "Deleting settings instance");
    delete pDBHandle;
    pDBHandle=NULL;
}

bool Settings::setVersion(int version)
{
    LOG4CXX_DEBUG(settingsLog, "Setting database version to " << version);
    try {
        pthread_mutex_lock( &settings_mutex );
        if (!pDBHandle->prepare("create table if not exists version (number INT)"))
        {
            LOG4CXX_ERROR(settingsLog, "Could not create version table: '" << pDBHandle->getLasterror().c_str() << "'");
            throw 1;
        }

        if (!pDBHandle->perform())
        {
            LOG4CXX_ERROR(settingsLog, "Create table version failed '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }
        pthread_mutex_unlock(&settings_mutex);
        int previousVersion = getVersion();
        pthread_mutex_lock( &settings_mutex );
        if (previousVersion == 0)
        {
            LOG4CXX_DEBUG(settingsLog, "insert into version " << version);
            if (!pDBHandle->prepare("insert into version values(?)"))
            {
                LOG4CXX_ERROR(settingsLog, "Could not prepare insert '" << pDBHandle->getLasterror() << "'");
                throw 1;
            }
        }
        else
        {
            LOG4CXX_DEBUG(settingsLog, "update version " << version);
            if (!pDBHandle->prepare("update version set number=?"))
            {
                LOG4CXX_ERROR(settingsLog, "Could not prepare update '" << pDBHandle->getLasterror() << "'");
                throw 1;
            }
        }

        if (!pDBHandle->bind(1, version))
        {
            LOG4CXX_ERROR(settingsLog, "Bind failed '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }

        if (!pDBHandle->perform())
        {
            LOG4CXX_ERROR(settingsLog, "Could not perform insert version number '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }
        pthread_mutex_unlock( &settings_mutex );
    } catch(int e) {
        LOG4CXX_TRACE(settingsLog, "Set version exception was thrown");
        pthread_mutex_unlock(&settings_mutex);
        return false;
    }


    return true;
}

int Settings::getVersion()
{
    int version = 0;
    try {
        pthread_mutex_lock( &settings_mutex );
        if (!pDBHandle->prepare("SELECT number FROM version"))
        {
            LOG4CXX_ERROR(settingsLog, "Could not read version: '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }

        DBResult result;
        if (!pDBHandle->perform(&result))
        {
            LOG4CXX_ERROR(settingsLog, "Query failed '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }

        while (result.loadRow())
        {
            version = result.getInt(0);
        }
        pthread_mutex_unlock( &settings_mutex );
    } catch(int e) {
        LOG4CXX_TRACE(settingsLog, "Get version exception was thrown");
        pthread_mutex_unlock(&settings_mutex);
        return 0;
    }


    return version;
}

ostream& operator <<(ostream& os, const SettingsItem& i)
{
    return os << i.getName() << '@' << i.getDomain() << '=' << i.getValue() << endl;;
}

// Loads a setting with key 'key' from database
bool Settings::read(SettingsItem &item, const string &_setting)
{
    string setting = _setting;
    Utils::trim(setting);

    if (pDBHandle == NULL)
    {
        LOG4CXX_ERROR(settingsLog, "Database not open");
        return false;
    }

    // Try to read the setting values
    try {
        pthread_mutex_lock( &settings_mutex );
        if (!pDBHandle->prepare("SELECT rowid, value, type FROM setting WHERE setting=? ORDER BY domain=?"))
        {
            LOG4CXX_ERROR(settingsLog, "Could not read setting: '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }

        LOG4CXX_TRACE(settingsLog, "Binding setting: '" << setting << "' in domain: " << mCurrentDomain);
        if (!pDBHandle->bind(1, setting.c_str()) || !pDBHandle->bind(2, mCurrentDomain.c_str()))
        {
            LOG4CXX_ERROR(settingsLog, "Bind failed '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }

        DBResult result;
        if (!pDBHandle->perform(&result))
        {
            LOG4CXX_ERROR(settingsLog, "Query failed '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }

        int count = 0;
        while (result.loadRow())
        {
            item.mRowid = result.getInt(0);
            item.mName = setting;
            item.mDomain = mCurrentDomain;
            item.mValue = result.getText(1);
            item.mType = (SettingsItem::Type) result.getInt(2);
            count++;
        }
        pthread_mutex_unlock( &settings_mutex );

        // Check if we got any rows
        if (count == 0)
            return false;

        pthread_mutex_lock( &settings_mutex );
        // Try to read the setting parameter values
        if (!pDBHandle->prepare("SELECT key, value, type FROM parameter WHERE setting_id=?"))
        {
            LOG4CXX_ERROR(settingsLog, "Could not read parameter: '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }

        if (!pDBHandle->bind(1, item.mRowid))
        {
            LOG4CXX_ERROR(settingsLog, "Bind failed '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }

        DBResult result2;
        if (!pDBHandle->perform(&result2))
        {
            LOG4CXX_ERROR(settingsLog, "Query failed '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }

        count = 0;
        while (result.loadRow())
        {
            string key = result.getText(0);
            string value = result.getText(0);
            item.mParameters[key] = value;
        }
        pthread_mutex_unlock( &settings_mutex );
    } catch(int e) {
        LOG4CXX_TRACE(settingsLog, "Read exception was thrown");
        pthread_mutex_unlock( &settings_mutex );
        return false;
    }

    return true;
}

// Overwrites values and parameters for a setting in database
bool Settings::write(SettingsItem &item)
{
    if (pDBHandle == NULL)
    {
        LOG4CXX_ERROR(settingsLog, "Database not open");
        return false;
    }

    // Try to insert the value
    try {
        pthread_mutex_lock( &settings_mutex );
        if (!pDBHandle->prepare("INSERT OR REPLACE INTO setting (setting, value, type, domain) VALUES (?,?,?,?)"))
        {
            LOG4CXX_ERROR(settingsLog, "Could not create/open setting table: '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }
        string name = item.getName();
        string value = item.getValue();
        if (!pDBHandle->bind(1, name.c_str()) || !pDBHandle->bind(2, value.c_str()) || !pDBHandle->bind(3, (int) item.getType()) || !pDBHandle->bind(4, mCurrentDomain.c_str()))
        {
            LOG4CXX_ERROR(settingsLog, "Bind failed '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }

        if (!pDBHandle->perform())
        {
            LOG4CXX_ERROR(settingsLog, "Query failed while inserting '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }
        pthread_mutex_unlock( &settings_mutex );
    } catch(int e) {
        LOG4CXX_TRACE(settingsLog, "Write exception was thrown");
        pthread_mutex_unlock( &settings_mutex );
        return false;
    }


    return true;
}

// Overwrites values and parameters for a setting in database
bool Settings::update(SettingsItem &item)
{
    if (pDBHandle == NULL)
    {
        LOG4CXX_ERROR(settingsLog, "Database not open");
        return false;
    }

    // Try to insert the value
    try {
        pthread_mutex_lock( &settings_mutex );
        if (!pDBHandle->prepare("UPDATE setting SET value=? WHERE rowid=?"))
        {
            LOG4CXX_ERROR(settingsLog, "Could not create update setting query: '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }

        if (!pDBHandle->bind(1, item.getValue().c_str()) || !pDBHandle->bind(2, item.getRowid()))
        {
            LOG4CXX_ERROR(settingsLog, "Bind failed '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }

        if (!pDBHandle->perform())
        {
            LOG4CXX_ERROR(settingsLog, "Query failed '" << pDBHandle->getLasterror() << "'");
            throw 1;
        }
        pthread_mutex_unlock( &settings_mutex );
    } catch(int e) {
        LOG4CXX_TRACE(settingsLog, "Write exception was thrown");
        pthread_mutex_unlock( &settings_mutex );
        return false;
    }


    return true;

}

bool Settings::restoreClosestDbBackup(string settingsfile)
{
    int v = DB_VERSION;
    for (; v >= 0; --v)
    {
        stringstream closestDbBackupFilename;
        closestDbBackupFilename << settingsfile << v;

        if (not fstream(closestDbBackupFilename.str().c_str(), fstream::in))
        {
            continue;
        }

        if (not filecopy(closestDbBackupFilename.str(), settingsfile))
        {
            LOG4CXX_ERROR(settingsLog, "Failed to restore backup settings.db");
            return false;
        }
        return true;
    }
    LOG4CXX_ERROR(settingsLog, "Failed to restore backup settings.db");
    return false;
}
