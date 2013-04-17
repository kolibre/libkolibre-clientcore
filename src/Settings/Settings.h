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

#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "../Utils.h"

#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <pthread.h>

class Settings;
class DB;

class SettingsItem
{
public:
    enum Type
    {
        type_unknown, type_double, type_bool, type_int, type_string
    };

    SettingsItem()
    {
        mName = "";
        mValue = "";
        mDomain = "local";
        mType = type_unknown;
        mRowid = -1;
    }
    ;

    template<class T> SettingsItem(const std::string &n, const T& v, const std::string &d)
    {
        mName = n;
        mValue = T_as_string(v);
        mType = get_type(v);
        mDomain = d;
        mRowid = -1;
    }

    void setName(const std::string &n)
    {
        std::string name = n;
        Utils::trim(name);
        mName = name;
    }
    ;

    std::string getName() const
    {
        return mName;
    }
    ;

    template<class T> void setValue(const T &v)
    {
        mValue = T_as_string(v);
        mType = get_type(v);
    }

    template<class T> T getValue() const
    {
        return string_as_T<T>(mValue);
    }

    std::string getValue() const
    {
        return mValue;
    }
    ;

    void setType(const Type &t)
    {
        mType = t;
    }
    ;

    Type getType() const
    {
        return mType;
    }
    ;

    void setDomain(const std::string &d)
    {
        mDomain = d;
    }
    ;

    std::string getDomain() const
    {
        return mDomain;
    }
    ;

    template<class T> void setParameter(std::string key, const T& value)
    {
        std::string val = T_as_string(value);
        Utils::trim(key);
        Utils::trim(val);
        mParameters[key] = val;
        return;
    }

    template<class T> T getParameter(const std::string& id)
    {
        std::string value = "";

        param_map::iterator it;

        it = mParameters.find(id);
        if (it != mParameters.end())
            value = it->second;

        return string_as_T<T>(value);
    }

    std::string getParameter(const std::string& id)
    {
        std::string value = "";

        param_map::iterator it;

        it = mParameters.find(id);
        if (it != mParameters.end())
            value = it->second;

        return value;
    }

    static Type get_type(const double &val)
    {
        return type_double;
    }
    ;

    static Type get_type(const bool &val)
    {
        return type_bool;
    }
    ;

    static Type get_type(const int &val)
    {
        return type_int;
    }
    ;

    static Type get_type(const std::string &val)
    {
        return type_string;
    }
    ;

    friend class Settings;

    //friend ostream& operator<< (const ostream &os, const SettingsItem &item);
    /*ostream& operator<< (ostream& os) {
     os << "name: " << mName << endl;
     os << "value: " << mValue << endl;
     };*/

    /*friend ostream& operator<< (ostream &os, SettingsItem item);
     {
     os << "name: " << mName << endl;
     os << "value: " << mValue << endl;
     };*/
    //param_map::iterator it
    //
    //for(it = mParameters.begin(); it != mParameters.end(); ++it) {
    //	os << " " << it->first << "=\"" << it->second << "\"";
    //}
    //}
    /*	string operator=(SettingsItem &item) {
     return mValue;
     }*/

    bool operator==(std::string name)
    {
        if (mName == name)
            return true;
        return false;
    }

private:
    std::string mName;
    std::string mValue;
    std::string mDomain;
    Type mType;
    long mRowid;

    long getRowid()
    {
        return mRowid;
    }
    ;

    template<class T> std::string T_as_string(const T& t) const;
    template<class T> T string_as_T(const std::string& s) const;
    typedef std::map<std::string, std::string> param_map;
    param_map mParameters;
};

class Settings
{
protected:
    Settings();
    template<class T> static std::string T_as_string(const T& t);
    template<class T> static T string_as_T(const std::string& s);

private:
    pthread_mutex_t settings_mutex;
    static Settings *pinstance;
    DB *pDBHandle;
    std::string mCurrentDomain;
    bool setVersion(int version);
    int getVersion();
    bool restoreClosestDbBackup(std::string settingsfile);
    ~Settings();

public:
    static Settings *Instance();
    void DeleteInstance();

    //template<class T> void setOption(std::string name, const T &value);
    //SettingsItem *getSettings(const std::string &key);

    bool read(SettingsItem &item, const std::string &setting);
    bool write(SettingsItem &item);
    bool update(SettingsItem &item);

    void setDomain(const std::string &domain)
    {
        mCurrentDomain = domain;
    }
    ;

    const std::string& getDomain() const
    {
        return mCurrentDomain;
    }
    ;

    // call as read<T>
    template<class T> T read(const std::string& setting);
    template<class T> T read(const std::string& setting, const T& defaultvalue);
    template<class T> bool write(const std::string& setting, const T& value);
    template<class T> bool readInto(T& var, const std::string& name);
    template<class T> bool readInto(T& var, const std::string& name, const T& defaultvalue);

    // thrown only by T read(key) variant of read()
    struct name_not_found
    {
        std::string name;
        name_not_found(const std::string& name_ = std::string()) :
                name(name_)
        {
        }
    };
};

template<class T>
std::string SettingsItem::T_as_string(const T& t) const
{
    // Convert from a T to a string
    // Type T must support << operator
    std::ostringstream ost;
    ost << t;
    return ost.str();
}

template<class T>
T SettingsItem::string_as_T(const std::string& s) const
{
    // Convert from a string to a T
    // Type T must support >> operator
    T t;
    std::istringstream ist(s);
    ist >> t;
    return t;
}

template<>
inline std::string SettingsItem::string_as_T<std::string>(const std::string& s) const
{
    // Convert from a string to a string
    // In other words, do nothing
    return s;
}

template<>
inline bool SettingsItem::string_as_T<bool>(const std::string& s) const
{
    // Convert from a string to a bool
    // Interpret "false", "F", "no", "n", "0" as false
    // Interpret "true", "T", "yes", "y", "1", "-1", or anything else as true
    bool b = true;
    std::string sup = s;
    for (std::string::iterator p = sup.begin(); p != sup.end(); ++p)
        *p = toupper(*p); // make string all caps
    if (sup == std::string("FALSE") || sup == std::string("F") || sup == std::string("NO") || sup == std::string("N") || sup == std::string("0") || sup == std::string("NONE"))
        b = false;
    return b;
}

/* static */
template<class T>
std::string Settings::T_as_string(const T& t)
{
    // Convert from a T to a string
    // Type T must support << operator
    std::ostringstream ost;
    ost << t;
    return ost.str();
}

/* static */
template<class T>
T Settings::string_as_T(const std::string& s)
{
    // Convert from a string to a T
    // Type T must support >> operator
    T t;
    std::istringstream ist(s);
    ist >> t;
    return t;
}

/* static */
template<>
inline std::string Settings::string_as_T<std::string>(const std::string& s)
{
    // Convert from a string to a string
    // In other words, do nothing
    return s;
}

/* static */
template<>
inline bool Settings::string_as_T<bool>(const std::string& s)
{
    // Convert from a string to a bool
    // Interpret "false", "F", "no", "n", "0" as false
    // Interpret "true", "T", "yes", "y", "1", "-1", or anything else as true
    bool b = true;
    std::string sup = s;
    for (std::string::iterator p = sup.begin(); p != sup.end(); ++p)
        *p = toupper(*p); // make string all caps
    if (sup == std::string("FALSE") || sup == std::string("F") || sup == std::string("NO") || sup == std::string("N") || sup == std::string("0") || sup == std::string("NONE"))
        b = false;
    return b;
}

// The two following reads userprofile settings
template<class T>
T Settings::read(const std::string& setting)
{
    SettingsItem i;
    if (read(i, setting))
        return string_as_T<T>(i.getValue());
    else
        throw name_not_found(setting);
}

// Return the value corresponding to key or 
// return defaultvalue if key is not found
template<class T>
T Settings::read(const std::string& setting, const T& defaultvalue)
{
    SettingsItem i;
    if (read(i, setting))
        return string_as_T<T>(i.getValue());
    else
        return defaultvalue;

}

// Replace the the value in the corresponding key 
// or create a new setting with the values if not found
template<class T>
bool Settings::write(const std::string& setting, const T& value)
{
    SettingsItem i;
    if (read(i, setting))
    {
        if (string_as_T<T>(i.getValue()) != value)
        {
            i.setValue(value);
            return update(i);
        }
        return true;
    }

    // setup a new SettingsItem
    i.setName(setting);
    i.setValue(value);
    i.setDomain(mCurrentDomain);

    return write(i);
}

template<class T> bool Settings::readInto(T& var, const std::string& setting)
{
    SettingsItem i;
    if (read(i, setting))
    {
        var = string_as_T<T>(i.getValue());
        return true;
    };
    return false;
}

template<class T> bool Settings::readInto(T& var, const std::string& setting, const T& defaultvalue)
{
    SettingsItem i;
    if (read(i, setting))
    {
        var = string_as_T<T>(i.getValue());
        return true;
    };
    var = defaultvalue;
    return false;
}

#endif // _SETTINGS_H
