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

#ifndef _KOLIBRE_H
#define _KOLIBRE_H

#include "NaviList.h"
#include "NaviListItem.h"

#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <boost/signals2.hpp>

#ifdef WIN32
#ifdef KOLIBRE_DLL
#define KOLIBRE_API __declspec(dllexport)
#else
#define KOLIBRE_API __declspec(dllimport)
#endif
#else
#define KOLIBRE_API
#endif

// data structures for events
/**
 * A data type to hold information about a book position
 */
struct BookPositionInfo
{
    /**
     * The current position in milliseconds
     *
     * Value is -1 if the position is unknown
     */
    long currentms;
    /**
     * The total duration in milliseconds
     *
     * Value is -1 if the duration in unknown
     */
    long totalms;
    BookPositionInfo(long now, long total) : currentms(now), totalms(total) {}
    BookPositionInfo() : currentms(-1), totalms(-1) {}
};

/**
 * A data type to hold information about a book page
 */
struct BookPageInfo
{
    /**
     * The id of the page
     *
     * Value is -1 if the id is unknown
     */
    int currentPageIdx;
    BookPageInfo(int idx) : currentPageIdx(idx) {}
    BookPageInfo() : currentPageIdx(-1) {}
};

/**
 * A data type to hold information about a book section
 */
struct BookSectionInfo
{
    /**
     * The id of the section
     *
     * Value is -1 if the id is unknown
     */
    int currentSectionIdx;
    BookSectionInfo(int idx) : currentSectionIdx(idx) {}
    BookSectionInfo() : currentSectionIdx(-1) {}
};

/**
 * A enumerated list of error types
 */
enum ErrorType
{
    NETWORK,
    SERVICE,
    CONTENT,
};

/**
 * A data type to hold information about an error
 */
struct ErrorMessage
{
    /**
     * The error type
     */
    ErrorType type_;
    /**
     * The error message
     */
    std::string message_;
    ErrorMessage(ErrorType type, std::string message) : type_(type), message_(message) {}
    ErrorMessage() : type_((ErrorType) -1), message_("") {}
};

/**
 * A data type to hold information about a daisy navigation level
 */
struct DaisyNaviLevel
{
    /**
     * The daisy navigation level
     */
    std::string level_;
    DaisyNaviLevel(std::string level) : level_(level) {}
    DaisyNaviLevel() : level_("") {}
};

// data structures for events end

enum SleepTimerStates
{
    SLEEP_TIMER_OFF = -1,
    SLEEP_TIMER_TIMED_OUT = 0,
    SLEEP_TIMER_NEAR_TIMEOUT = 30,
    SLEEP_TIMER_ON = 31
};

/**
 * The core class for the client library.
 */
class KOLIBRE_API ClientCore
{
public:
    ClientCore(const std::string service_url = "https://daisy.kolibre.com/daisyonline/service.php", const std::string useragent = "");
    ~ClientCore();

    enum COMMAND
    {
        HOME,                           /**< Go to the root node */
        UP,                             /**< Go to parent node */
        DOWN,                           /**< Open selected child node */
        LEFT,                           /**< Go to previous child node */
        RIGHT,                          /**< Go to next child node */
        BACK,                           /**< Go to parent node or back in history during playback */
        EXIT,                           /**< Exit the application */
        PAUSE,                          /**< Open selected child node or pause the playback */
        BOOKMARK,                       /**< Enter bookmark management */
        CONTEXTMENU,                    /**< Open context menu */
        HELP,                           /**< Start playing the help audio */
        HELP_CLOSE,                     /**< Stop playing the help audio */
        ABOUT,                          /**< Start playing the about audio */
        ABOUT_CLOSE,                    /**< Stop playing the about audio */
        RETRY_LOGIN,                    /**< Retry the login procedure for a Daisy Online service */
        SPEEDDOWN,                      /**< Decrement reading speed one step */
        SPEEDUP,                        /**< Increment reading speed one step */
        SLEEP_OFF,                      /**< Deactivate the sleep timer */
        SLEEP_15,                       /**< Activate the sleep timer with 15 minutes */
        SLEEP_30,                       /**< Activate the sleep timer with 30 minutes */
        SLEEP_60,                       /**< Activate the sleep timer with 60 minutes */
        NARRATOR_CONTROL_STOP,          /**< Take control of narrator */
        NARRATOR_CONTROL_CONTINUE,      /**< Give back control of narrator */
        BOOKINFO,                       /**< Open book info menu in the context menu*/
        GOTO_TIME,                      /**< Open go-to time menu in the context menu */
        GOTO_PERCENT,                   /**< Open go-to percent menu in the context menu */
        GOTO_PAGE,                      /**< Open go-to page menu in the context menu */
    };

    // player/narrator control functions
    void setSpeedPercent(int percent);
    void narratorFinished();

    // handle online and offline sources
    int addDaisyOnlineService(std::string name, std::string url, std::string username, std::string password, bool rememberPassword = false);
    int addFileSystemPath(std::string name, std::string path);

    // getters and setters
    void setManualSound(const char *);
    std::string getManualSound();
    void setAboutSound(const char *);
    std::string getAboutSound();
    void setServiceUrl(const std::string url);
    std::string getServiceUrl();
    std::string getUserAgent();
    void setUsername(const std::string username);
    std::string getUsername();
    void setPassword(const std::string password, bool remember);
    std::string getPassword();
    bool getRememberPassword();
    void setSerialNumber(const std::string serialNumber);
    std::string getSerialNumber();
    static void setLanguage(std::string lang);
    static std::string getLanguage();

    // sleep timer functions
    void setSleepTimerTimeLeft(int minutes);
    int getSleepTimerTimeLeft();
    void setSleepTimerState(SleepTimerStates state);
    SleepTimerStates getSleepTimerState();
    int getSleepTimerSetting();

    // operate functions
    void pushCommand(COMMAND command);
    bool jumpToSecond(unsigned int second);
    bool jumpToUri(const std::string uri);
    bool isRunning();
    void start();
    void shutdown();

    /**
     * A data type to hold information about a book
     */
    struct BookData
    {
        /**
         * A data type to hold information about a page
         */
        struct Page
        {
            Page(const std::string& ref, const std::string& text, int playOrder) :
                    ref(ref), text(text), playOrder(playOrder)
            {
            }

            /**
             * The unique uri for this page
             */
            std::string ref;

            /**
             * The name for the page
             */
            std::string text;

            /**
             * The order when this page is played
             */
            int playOrder;
        };

        /**
         * A data type to hold information about a section
         */
        struct Section
        {
            Section(const std::string& ref, const std::string& text, int playOrder, int level) :
                    ref(ref), text(text), playOrder(playOrder), level(level)
            {
            }

            /**
             * The unique uri fot this section
             */
            std::string ref;

            /**
             * The name for the section
             */
            std::string text;

            /**
             * The order when this section is played
             */
            int playOrder;

            /**
             * The navigation level this section belongs to
             */
            int level;
        };

        /**
         * Vector containing book pages
         */
        std::vector<Page> pages;

        /**
         * Vector containing book sections
         */
        std::vector<Section> sections;
    };

    // signals and slots
    /**
     * Info timer timeout events are emitted via this signal
     */
    boost::signals2::signal<void()> infoTimeout_signal;
    /**
     * Sleep time timeout event is emitted via this signal
     */
    boost::signals2::signal<void()> sleepTimeout_signal;
    /**
     * Loging results are emitted via this signal
     */
    boost::signals2::signal<void(bool)> loginResult_signal;
    /**
     * Command Queue empty is emitted via this signal
     */
    boost::signals2::signal<void()> queueEmpty_signal;
    /**
     * Error messages are emitted via this signal
     */
    boost::signals2::signal<void(ErrorMessage)> errorMessage_signal;
    /**
     * Book data is emitted via this signal
     */
    boost::signals2::signal<void(BookData)> bookData_signal;
    /**
     * Book position changes are emitted via this signal
     */
    boost::signals2::signal<void(BookPositionInfo)> bookPosition_signal;
    /**
     * Book page changes are emitted via this signal
     */
    boost::signals2::signal<void(BookPageInfo)> bookPage_signal;
    /**
     * Book sction cahnges are emitted via this signal
     */
    boost::signals2::signal<void(BookSectionInfo)> bookSection_signal;
    /**
     * Daisy navigation level changes are emitted via this signal
     */
    boost::signals2::signal<void(DaisyNaviLevel)> daisyNaviLevel_signal;
    /**
     * Navigation list changes are emitted via this signal
     */
    boost::signals2::signal<void(NaviList)> naviList_signal;
    /**
     * Navigation item changes are emitted via this signal
     */
    boost::signals2::signal<void(NaviListItem)> naviListItem_signal;
    // signals and slots end

private:
    static void *clientcore_thread(void *ctx);

    pthread_mutex_t clientcoreMutex;
    pthread_t clientcoreThread;
    bool clientcoreRunning;
    bool threadStarted;
    time_t sleepTimerStart;
    time_t sleepTimerEnd;
    int sleepTimerSetting;
    int sleepTimerState;
    std::string mManualOggfile;
    std::string mAboutOggfile;
    std::string mServiceUrl;
    std::string mUsername;
    std::string mPassword;
    std::string mDownloadFolder;
    std::string mUserAgent;
    std::string mSerialNumber;

    /**
     * A data type to hold information about a DaisyOnline service
     */
    struct DaisyOnlineService
    {
        DaisyOnlineService(std::string name, std::string url, std::string username, std::string password, bool remember) :
            name(name), url(url), username(username), password(password), rememberPassword(remember)
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
         * Boolean the determine if we shall remember the password for the service
         */
        bool rememberPassword;
    };

    /**
     * A data type to hold information about a file system path
     */
    struct FileSystemPath
    {
        FileSystemPath(std::string name, std::string path) :
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

#endif
