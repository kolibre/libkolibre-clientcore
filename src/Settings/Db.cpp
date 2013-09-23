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

#ifdef WIN32
#include <iconv.h>
#include <windows.h>
#endif

#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <log4cxx/logger.h>


using namespace std;

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr dbLog(log4cxx::Logger::getLogger("kolibre.clientcore.db"));

int busyHandler(void *pArg1, int iPriorCalls)
{
    LOG4CXX_WARN(dbLog, "busyHandler " << iPriorCalls << " !!");
    // sleep if handler has been called less than threshold value
    if (iPriorCalls < 20)
    {
        // adding a random value here greatly reduces locking
        if (pArg1 < 0)
            usleep((rand() % 500000) + 400000);
        else
            usleep(500000);
        return 1;
    }

    // have sqlite3_exec immediately return SQLITE_BUSY
    return 0;
}

namespace settings {

DB::DB(const string &database):
    mDatabase(database), pDBHandle(NULL), pStatement(NULL), mLasterror(""), mLastquery("")
{

}

DB::DB(sqlite3 *handle)
{
    pStatement = NULL;
    pDBHandle = handle;
    int sleepMode = 1;
    sqlite3_busy_handler(pDBHandle, &busyHandler, &sleepMode);
}

bool DB::connect()
{
    if (pDBHandle != NULL)
        return true;

    LOG4CXX_DEBUG(dbLog, "Opening settings database: " << mDatabase);
    if (sqlite3_open_v2(mDatabase.c_str(), &pDBHandle, SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK)
    {
        mLasterror.assign(sqlite3_errmsg(pDBHandle));
        sqlite3_close(pDBHandle);
        pDBHandle = NULL;
        return false;
    }

    int sleepMode = 1;
    sqlite3_busy_handler(pDBHandle, &busyHandler, &sleepMode);

    bClosedb = true;
    return true;
}

DB::~DB()
{
    // If we recieved the handle in the constructor the caller should close the db
    if (pDBHandle && bClosedb)
    {
        sqlite3_close(pDBHandle);
    }
}

bool DB::prepare(const char *query)
{
    if (!pDBHandle)
        return false;
    if (pStatement)
        sqlite3_finalize(pStatement);

    if (query == NULL)
        query = mLastquery.c_str();

    pStatement = NULL;
    rc = sqlite3_prepare_v2(pDBHandle, query, -1, &pStatement, 0);
    if (rc != SQLITE_OK)
    {
        mLasterror.assign(sqlite3_errmsg(pDBHandle));
        return false;
    }

    mLastquery.assign(query);
    return true;
}

bool DB::bind(const int idx, const int value)
{
    if (!pStatement)
        prepare(NULL);
    if (idx == 0)
        return false;
    rc = sqlite3_bind_int(pStatement, idx, value);
    if (rc != SQLITE_OK)
    {
        mLasterror.assign(sqlite3_errmsg(pDBHandle));
        return false;
    }
    return true;
}

bool DB::bind(const char *key, const int value)
{
    return bind(sqlite3_bind_parameter_index(pStatement, key), value);
}

bool DB::bind(const int idx, const long value)
{
    if (!pStatement)
        prepare(NULL);
    if (idx == 0)
        return false;
    rc = sqlite3_bind_int(pStatement, idx, value);
    if (rc != SQLITE_OK)
    {
        mLasterror.assign(sqlite3_errmsg(pDBHandle));
        return false;
    }
    return true;
}

bool DB::bind(const char *key, const long value)
{
    return bind(sqlite3_bind_parameter_index(pStatement, key), value);
}

bool DB::bind(const int idx, const double value)
{
    if (!pStatement)
        prepare(NULL);
    if (idx == 0)
        return false;
    rc = sqlite3_bind_double(pStatement, idx, value);
    if (rc != SQLITE_OK)
    {
        mLasterror.assign(sqlite3_errmsg(pDBHandle));
        return false;
    }
    return true;
}

bool DB::bind(const char *key, const double value)
{
    return bind(sqlite3_bind_parameter_index(pStatement, key), value);
}

bool DB::bind(const int idx, const char *value, int length, void (*freefunc)(void*))
{
    if (!pStatement)
        prepare(NULL);
    if (idx == 0)
        return false;
    rc = sqlite3_bind_text(pStatement, idx, value, length, freefunc);
    if (rc != SQLITE_OK)
    {
        mLasterror.assign(sqlite3_errmsg(pDBHandle));
        return false;
    }
    return true;
}

bool DB::bind(const char *key, const char *value, int length, void (*freefunc)(void*))
{
    return bind(sqlite3_bind_parameter_index(pStatement, key), value, length, freefunc);
}

bool DB::bind(const int idx, const void *value, int length, void (*freefunc)(void*))
{
    if (!pStatement)
        prepare(NULL);
    if (idx == 0)
        return false;
    rc = sqlite3_bind_blob(pStatement, idx, value, length, freefunc);
    if (rc != SQLITE_OK)
    {
        mLasterror.assign(sqlite3_errmsg(pDBHandle));
        return false;
    }
    return true;
}

bool DB::bind(const char *key, const void *value, int length, void (*freefunc)(void*))
{
    return bind(sqlite3_bind_parameter_index(pStatement, key), value, length, freefunc);
}

bool DB::reset()
{
    if (pStatement)
    {
        rc = sqlite3_reset(pStatement);
        if (rc != SQLITE_OK)
        {
            mLasterror.assign(sqlite3_errmsg(pDBHandle));
            return false;
        }
    }
    // Prepares a copy of the previous query
    return prepare(NULL);
}

bool DB::perform(DBResult *result)
{
    if (!pStatement)
        return false;

    // Called does not want a result, only success or failure
    if (result == NULL)
    {
        DBResult res;
        res.setup(pDBHandle, pStatement);
        pStatement = NULL;
        while (!res.isError() && !res.isDone() && res.loadRow());
        bool ret = (!res.isError() && res.isDone());

        //printf("in perform, result NULL, returning %i, err %i, done %i\n", ret, res.isError(), res.isDone());

        return ret;
    }

    // return a result
    bool ret = result->setup(pDBHandle, pStatement);
    pStatement = NULL;
    return ret;
}

DBResult::DBResult()
{
    pDBHandle = NULL;
    pStatement = NULL;
}

bool DBResult::setup(sqlite3 *handle, sqlite3_stmt* statement)
{
    pDBHandle = handle;
    pStatement = statement;
    bFirstcall = true;
    bError = false;
    bDone = false;

    // Step once
    step();

    return !bError;
}

DBResult::~DBResult()
{
    if (pStatement)
    {
        sqlite3_finalize(pStatement);
    }
}

int DBResult::step()
{
    rc = sqlite3_step(pStatement);
    switch (rc)
    {
    case SQLITE_OK:
        //printf("Got ok\n");
        return rc;
        break;

    case SQLITE_ROW:
        //printf("Got row\n");
        return rc;

    case SQLITE_DONE:
        bDone = true;
        //printf("Got done\n");
        return 0;

    case SQLITE_CONSTRAINT:
        LOG4CXX_ERROR(dbLog, "Got constraint violation");
        bError = true;
        mLasterror.assign("DB: SQLITE_CONSTRAINT violation");
        return rc;
        break;

    case SQLITE_MISUSE:
        bError = true;
        mLasterror.assign("DB: SQLITE_MISUSE");
        return rc;
        break;

    default:
        LOG4CXX_ERROR(dbLog, "DB: error code: " << rc);
        bError = true;
        mLasterror = sqlite3_errmsg(pDBHandle);
    }
    return rc;
}

bool DBResult::isError()
{
    return bError;
}

bool DBResult::isDone()
{
    return bDone;
}

bool DBResult::loadRow()
{
    if (bDone)
        return false;
    if (bError)
        return false;
    if (bFirstcall)
    {
        bFirstcall = false;
        if (rc == SQLITE_ROW)
            return true;
        return false;
    }

    if (step() == SQLITE_ROW)
        return true;
    return false;
}

void DBResult::printRow()
{
    int i;
    for (i = 0; i < sqlite3_column_count(pStatement); ++i)
        switch (sqlite3_column_type(pStatement, i))
        {
        case SQLITE_INTEGER:
            printf("%d\t", sqlite3_column_int(pStatement, i));
            break;
        case SQLITE_FLOAT:
            printf("%f\t", sqlite3_column_double(pStatement, i));
            break;
        case SQLITE_TEXT:
            printf("%s\t", sqlite3_column_text(pStatement, i));
            break;
        case SQLITE_BLOB:
            printf("<blob>%X\t", sqlite3_column_blob(pStatement, i));
            break;
        case SQLITE_NULL:
            printf("Null ");
            break;
        default:
            printf(" *Cannot determine SQLITE TYPE* col=%d ", i);
        }
    printf("\n");
}

long DBResult::getInsertId()
{
    return sqlite3_last_insert_rowid(pDBHandle);
}

long DBResult::getInt(const long column)
{
    if (sqlite3_column_type(pStatement, column) == SQLITE_INTEGER)
        return sqlite3_column_int(pStatement, column);

    return 0;
}

double DBResult::getDouble(const long column)
{
    if (sqlite3_column_type(pStatement, column) == SQLITE_FLOAT)
        return sqlite3_column_double(pStatement, column);

    return 0;
}

const char *DBResult::getText(const long column)
{
    if (sqlite3_column_type(pStatement, column) == SQLITE_TEXT)
        return (const char *) sqlite3_column_text(pStatement, column);

    return NULL;
}

const void *DBResult::getData(const long column)
{
    if (sqlite3_column_type(pStatement, column) == SQLITE_BLOB)
        return sqlite3_column_blob(pStatement, column);

    return NULL;
}

long DBResult::getDataSize(const long column)
{
    if (sqlite3_column_type(pStatement, column) == SQLITE_TEXT || sqlite3_column_type(pStatement, column) == SQLITE_BLOB)
        return sqlite3_column_bytes(pStatement, column);

    return 0;
}

}
