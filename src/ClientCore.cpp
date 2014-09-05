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

#include "ClientCore.h"
#include "MediaSourceManager.h"
#include "RootNode.h"
#include "Defines.h"
#include "Navi.h"
#include "Utils.h"
#include "version.h"
#include "config.h"
#include "Commands/NotifyCommands.h"
#include "Commands/InternalCommands.h"
#include "Commands/JumpCommand.h"
#include "CommandQueue2/CommandQueue.h"
#include "Settings/Settings.h"

#include <Narrator.h>
#include <Player.h>
#include <DataStreamHandler.h>

#include <unistd.h>
#include <iostream>
#include <boost/regex.hpp>
#include <libintl.h>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr clientcoreLog(log4cxx::Logger::getLogger("kolibre.clientcore.clientcore"));

using namespace std;

#define INFO_TIMEOUT 10;

/**
 * Constructor
 *
 * @param useragent The user-agent property in HTTP headers this application shall use
 */
ClientCore::ClientCore(const std::string useragent)
{
    LOG4CXX_INFO(clientcoreLog, VERSION_PACKAGE_NAME << " " << VERSION_PACKAGE_VERSION << " built " << __DATE__ << " " << __TIME__);

    // If no versioning info comes from above, use package name and version as UserAgent
    // If UserAgent information info comes from above adhere to that
    if (useragent == "")
        mUserAgent = std::string(VERSION_PACKAGE_NAME) + "/" + std::string(VERSION_PACKAGE_VERSION);
    else
        mUserAgent = useragent;

    // Initialize support classes
    LOG4CXX_INFO(clientcoreLog, "Setting up MediaSourceManager");
    MediaSourceManager *manager = MediaSourceManager::Instance();

    LOG4CXX_INFO(clientcoreLog, "Setting up DataStreamHandler")
    DataStreamHandler::Instance()->setUseragent(useragent);

    LOG4CXX_INFO(clientcoreLog, "Setting up Settings");
    Settings *settings = Settings::Instance();

    LOG4CXX_INFO(clientcoreLog, "Setting up Player");
    Player *player = Player::Instance();
    player->setUseragent(useragent);
    if (player->enable(NULL, NULL))
    { //&argc, &argv)) {
        LOG4CXX_ERROR(clientcoreLog, "Failed to enable player");
        return;
    }
    player->setTempo(settings->read<double>("playbackspeed", 1.0));

    LOG4CXX_INFO(clientcoreLog, "Setting up Narrator");
    Narrator *narrator = Narrator::Instance();
    std::string messagedb = Utils::getDatapath() + "messages.db";
    narrator->setDatabasePath(messagedb);
    narrator->connectAudioFinished(boost::bind(&ClientCore::narratorFinished, this));
    narrator->setTempo(settings->read<double>("playbackspeed", 1.0));
    narrator->setLanguage(settings->read<std::string>("language", "sv"));

    bindtextdomain(PACKAGE, "locale");

    // Initialize mutex
    pthread_mutex_init(&clientcoreMutex, NULL);

    // Initialize sleep timer
    setSleepTimerState(SLEEP_TIMER_OFF);

    // Initialize thread variables
    clientcoreRunning = false;
    threadStarted = false;

    // Initialize class member variables
    mManualOggfile = "";
    mAboutOggfile = "";
    mDownloadFolder = "";
}

/**
 * Destructor
 */
ClientCore::~ClientCore()
{
    pthread_mutex_lock(&clientcoreMutex);
    clientcoreRunning = false;
    pthread_mutex_unlock(&clientcoreMutex);

    if (threadStarted)
    {
        // Wait until ClientCore thread stops
        LOG4CXX_INFO(clientcoreLog, "Waiting for clientcoreThread to join");
        pthread_join(clientcoreThread, NULL);
    }

    LOG4CXX_DEBUG(clientcoreLog, "Deleting Settings");
    Settings::Instance()->DeleteInstance();

    LOG4CXX_DEBUG(clientcoreLog, "Deleting MediaSourceManager")
    MediaSourceManager::Instance()->DeleteInstance();
}

/**
 * Set the reading speed by percent
 * @param percent The desired reading speed
 */
void ClientCore::setSpeedPercent(int percent)
{
    // calculate minimum/maximum common tempo
    double minTempo = std::min(NARRATOR_MIN_TEMPO, PLAYER_MIN_TEMPO);
    double maxTempo = std::max(NARRATOR_MAX_TEMPO, PLAYER_MAX_TEMPO);

    // calculate the tempo change
    float zoneSize = 10 / (maxTempo - minTempo); // about 6.67 with max 2 and min 0.5
    float zeroIndexTempo = floor(percent / zoneSize) / 10; // between 0 and 1.5 with above values
    float tempo = zeroIndexTempo + minTempo;

    LOG4CXX_DEBUG(clientcoreLog, "Setting tempo to " << percent << "% which corresponds to " << tempo);

    Player::Instance()->setTempo(tempo);
    Narrator::Instance()->setTempo(tempo);
    Settings::Instance()->write<double>("playbackspeed", tempo);
}

/**
 * Function to be invoked on narrator finished signals, do not invoke manually.
 */
void ClientCore::narratorFinished()
{
    cq2::Command<INTERNAL_COMMAND> c(COMMAND_NARRATORFINISHED);
    c();
}

/**
 * Add a DaisyOnline service
 *
 * @param name The name of the service to distinguish it from other services
 * @param url The service URL
 * @param username Username for the service
 * @param password Password for the service
 * @param rememberPassword Parameter indicating whether to store the password or not
 * @return The index of the added service, or -1 if the service was not added
 */
int ClientCore::addDaisyOnlineService(std::string name, std::string url, std::string username, std::string password, bool rememberPassword)
{
    return MediaSourceManager::Instance()->addDaisyOnlineService(name, url, username, password, rememberPassword);
}

/**
 * Add a file system path
 *
 * @param name The name of the path to distinguish it from other paths
 * @param path The path on the file system
 * @return The index of the added path, or -1 if the path was not added
 */
int ClientCore::addFileSystemPath(std::string name, std::string path)
{
    return MediaSourceManager::Instance()->addFileSystemPath(name, path);
}

/**
 * Set the file which shall be played when command HELP is received
 *
 * @param manualsound A pointer to the manual audio file (must be an .ogg file)
 */
void ClientCore::setManualSound(const char *manualsound)
{
    LOG4CXX_DEBUG(clientcoreLog, "setting manual sound to '" << manualsound << "'");
    pthread_mutex_lock(&clientcoreMutex);
    mManualOggfile = manualsound;
    pthread_mutex_unlock(&clientcoreMutex);
}

/**
 * Get the manual audio file name
 *
 * @return The file name for the manual audio
 */
std::string ClientCore::getManualSound()
{
    pthread_mutex_lock(&clientcoreMutex);
    std::string manual = mManualOggfile;
    pthread_mutex_unlock(&clientcoreMutex);
    return manual;
}

/**
 * Set the file which shall be played when command ABOUT is received
 *
 * @param aboutsound A pointer to the about audio file (must be an .ogg file)
 */
void ClientCore::setAboutSound(const char *aboutsound)
{
    LOG4CXX_DEBUG(clientcoreLog, "Setting about sound to '" << aboutsound << "'");
    pthread_mutex_lock(&clientcoreMutex);
    mAboutOggfile = aboutsound;
    pthread_mutex_unlock(&clientcoreMutex);
}

/**
 * Get the about audio file name
 *
 * @return The file name for the manual audio
 */
std::string ClientCore::getAboutSound()
{
    pthread_mutex_lock(&clientcoreMutex);
    std::string about = mAboutOggfile;
    pthread_mutex_unlock(&clientcoreMutex);
    return about;
}

/**
 * Set the language to be used by this application
 *
 * @param lang A string conforming to the ISO 639 specification
 */
void ClientCore::setLanguage(std::string lang)
{
    // test if language string is an ISO-639 string
    boost::cmatch m;
    boost::regex e("^[a-zA-Z]{2}(-[a-zA-Z0-9]{2,3})*$");

    if (boost::regex_match(lang.c_str(), m, e))
    {
        // only use the first two letters in lower case
        std::string langCode = Utils::toLower(lang.substr(0, 2));
        Settings *settings = Settings::Instance();
        settings->write<std::string>("language", langCode);
        LOG4CXX_DEBUG(clientcoreLog, "Setting language to '" << langCode << "'");
    }
    else
    {
        LOG4CXX_WARN(clientcoreLog, "'" << lang << "' is not an ISO 639 string, please use something like en-EN instead");
    }
}

/**
 * Get the language used by this application
 *
 * @return A two letter ISO 639 language code
 */
std::string ClientCore::getLanguage()
{
    Settings *settings = Settings::Instance();
    return settings->read<std::string>("language", "unknown");
}

/**
 * Set the url to a service which this application shall connect to
 *
 * @param url A Daisy Online service url
 */
void ClientCore::setServiceUrl(const std::string url)
{
    // Note. This function will become deprecated when support for more services is implemented.
    MediaSourceManager::Instance()->setDOSurl(0, url);
}

/**
 * Get the service which this application shall connect to
 *
 * @return A url to a Daisy Online service
 */
std::string ClientCore::getServiceUrl()
{
    // Note. This function will become deprecated when support for more services is implemented.
    return MediaSourceManager::Instance()->getDOSurl(0);
}

/**
 * Get the user-agent value used by this application
 *
 * @return The user-agent property
 */
std::string ClientCore::getUserAgent()
{
    pthread_mutex_lock(&clientcoreMutex);
    std::string useragent = mUserAgent;
    pthread_mutex_unlock(&clientcoreMutex);
    return useragent;
}

/**
 * Set the username for the service to connect to
 *
 * @param username The username for the service
 */
void ClientCore::setUsername(const std::string username)
{
    // Note. This function will become deprecated when support for more services is implemented.
    MediaSourceManager::Instance()->setDOSusername(0, username);
}


/**
 * Set the password for the service to connect to
 *
 * @param password The password for the service
 * @param remember Inform whether the password shall be remembered or not
 */
void ClientCore::setPassword(const std::string password, bool remember)
{
    // Note. This function will become deprecated when support for more services is implemented.
    MediaSourceManager::Instance()->setDOSpassword(0, password);
    MediaSourceManager::Instance()->setDOSremember(0, remember);
}

/**
 * Get the username for the service to connect to
 *
 * @return The username
 */
std::string ClientCore::getUsername()
{
    // Note. This function will become deprecated when support for more services is implemented.
    return MediaSourceManager::Instance()->getDOSusername(0);
}

/**
 * Get the password for the service to connect to
 *
 * @return The password password
 */
std::string ClientCore::getPassword()
{
    // Note. This function will become deprecated when support for more services is implemented.
    return MediaSourceManager::Instance()->getDOSpassword(0);
}

/**
 * Set the serial number for the core library
 *
 * @param serialnumber The serial number string
 */
void ClientCore::setSerialNumber(const std::string serialnumber)
{
    pthread_mutex_lock(&clientcoreMutex);
    mSerialNumber = serialnumber;
    pthread_mutex_unlock(&clientcoreMutex);
    Settings::Instance()->write<std::string>("serialnumber", serialnumber);
}

/**
 * Get the serial number used by the core library
 *
 * @return The serial number
 */
std::string ClientCore::getSerialNumber()
{
    pthread_mutex_lock(&clientcoreMutex);
    std::string serialnumber = mSerialNumber;
    pthread_mutex_unlock(&clientcoreMutex);
    return serialnumber;
}

/**
 * Get the number of minutes the sleep timer was activated with
 *
 * @return The current setting
 * @retval -1 indicates that the sleep timer is not activated
 */
int ClientCore::getSleepTimerSetting()
{
    return sleepTimerSetting;
}

/**
 * Get the number of seconds left on the sleep timer
 *
 * @return Number of seconds left
 * @retval -1 indicates that the sleep timer is not activated
 */
int ClientCore::getSleepTimerTimeLeft()
{
    if (sleepTimerState == SLEEP_TIMER_OFF)
        return -1;

    return sleepTimerEnd - time(NULL);
}

/**
 * Activate the sleep timer
 *
 * @param minutes The number a minutes to activate the sleep timer with (deactivated with a non-positive value)
 */
void ClientCore::setSleepTimerTimeLeft(int minutes)
{
    Narrator *narrator = Narrator::Instance();
    Player *player = Player::Instance();

    if (minutes == -1)
    {
        setSleepTimerState(SLEEP_TIMER_OFF);
        LOG4CXX_INFO(clientcoreLog, "SleepTimer turned OFF");
        narrator->stop();
        player->pause();
        narrator->play(_N("sleep timer disabled"));
    }
    else if (minutes > 0)
    {
        setSleepTimerState(SLEEP_TIMER_ON);
        sleepTimerStart = time(NULL);
        sleepTimerEnd = sleepTimerStart + minutes * 60;
        sleepTimerSetting = minutes;
        LOG4CXX_INFO(clientcoreLog, "SleepTimer set to " << minutes << " minutes");
        narrator->stop();
        player->pause();
        narrator->play(_N("sleep timer enabled"));
        narrator->playDuration(0, minutes, 0);
    }
}

/**
 * Set the state for the sleep timer
 *
 * @param state An enumerated state value
 */
void ClientCore::setSleepTimerState(SleepTimerStates state)
{
    sleepTimerState = state;
    switch (state)
    {
    case SLEEP_TIMER_OFF:
        sleepTimerStart = 0;
        sleepTimerEnd = 0;
        sleepTimerSetting = -1;
        break;
    default:
        break;
    }
}

/**
 * Get the state for the sleep timer
 *
 * @return The current state
 */
SleepTimerStates ClientCore::getSleepTimerState()
{
    return (SleepTimerStates) sleepTimerState;
}

/**
 * Get the status whether to remember password or not
 *
 * @return A boolean indicating whether the password is remembered or not
 */
bool ClientCore::getRememberPassword()
{
    // Note. This function will become deprecated when support for more services is implemented.
    return MediaSourceManager::Instance()->getDOSremember(0);
}

/**
 * Execute a command
 *
 * @param command The command to execute
 */
void ClientCore::pushCommand(COMMAND command)
{
    // only a push command can deactivate the sleep timer
    if (sleepTimerState == SLEEP_TIMER_NEAR_TIMEOUT)
    {
        LOG4CXX_INFO(clientcoreLog, "SleepTimer deactivated by user");
        setSleepTimerTimeLeft(-1);
        Player::Instance()->resume();
    }

    cq2::Command<ClientCore::COMMAND> c(command);
    c();
}

/**
 * Jump to second in a digital talking book
 *
 * @param second The targeted second to jump to
 *
 * @return Always return true
 */
bool ClientCore::jumpToSecond(unsigned int second)
{
    Narrator::Instance()->play(_N("jumping to"));
    Narrator::Instance()->playDuration(second);
    cq2::Command<JumpCommand<unsigned int> > jump(second);
    jump();
    return true;
}

/**
 * Jump to a uri in the current navigation list (menu or digital talking book)
 *
 * @param uri The unique uri to jump to
 *
 * @return Always return true
 */
bool ClientCore::jumpToUri(const std::string uri)
{
    Narrator::Instance()->stop();
    cq2::Command<JumpCommand<std::string> > jump(uri);
    jump();
    return true;
}

/**
 * Get the status the application
 *
 * @return A boolean indicating if the application is running or not
 */
bool ClientCore::isRunning()
{
    bool running = false;
    pthread_mutex_lock(&clientcoreMutex);
    running = clientcoreRunning;
    pthread_mutex_unlock(&clientcoreMutex);
    return running;
}

/**
 * Start the application
 */
void ClientCore::start()
{
    // Start main thread
    LOG4CXX_INFO(clientcoreLog, "Setting up clientcoreThread");
    if (pthread_create(&clientcoreThread, NULL, &ClientCore::clientcore_thread, this))
    {
        LOG4CXX_FATAL(clientcoreLog, "Failed to start clientcoreThread");
        usleep(500000);
        clientcoreRunning = false;
    }
    clientcoreRunning = true;
    threadStarted = true;
}

/**
 * Shut down and exit the application
 */
void ClientCore::shutdown()
{
    pthread_mutex_lock(&clientcoreMutex);
    clientcoreRunning = false;
    pthread_mutex_unlock(&clientcoreMutex);
}

// struct ClientCoreState (read global variables)
// The handlers operate on this state, only for internal use
struct ClientCoreState
{
    time_t infoTimer;
    bool bHelpmode;
    bool retryLogin;
    bool commandQueueEmpty;
    bool sleepTimerTimeout;

    ClientCoreState() :
            bHelpmode(false),
            retryLogin(false),
            commandQueueEmpty(true),
            sleepTimerTimeout(false)
    {
        infoTimer = time(NULL) + INFO_TIMEOUT;
    }
};

// command handlers

struct Handle_ClientCoreCommands: public cq2::Handler<ClientCore::COMMAND>
{
    Handle_ClientCoreCommands(Navi* navi, ClientCore* clientcore, ClientCoreState* state) :
            navi_(navi), clientcore_(clientcore), state_(state)
    {
    }

private:
    Navi* navi_;
    ClientCore* clientcore_;
    ClientCoreState* state_;

    void handle(ClientCore::COMMAND command)
    {
        Narrator* narrator = Narrator::Instance();
        Player* player = Player::Instance();
        Settings* settings = Settings::Instance();

        switch (command)
        {
        case ClientCore::EXIT:
        {
            LOG4CXX_INFO(clientcoreLog, "ClientCore::EXIT received");
            if (state_->retryLogin)
            {
                usleep(50000); while (Narrator::Instance()->isSpeaking()) usleep(100000);
            }
            clientcore_->shutdown();
        }
            break;

        case ClientCore::HELP:
        {
            LOG4CXX_INFO(clientcoreLog, "ClientCore::HELP received");
            narrator->stop();
            player->pause();

            state_->bHelpmode = true;

            std::string manual = clientcore_->getManualSound();
            if (manual != "")
            {
                narrator->playFile(manual);
                // Wait a while, then stop player so that it doesn't start when narrator finishes
                usleep(500000);
                player->stop();
            }
            else
            {
                LOG4CXX_ERROR(clientcoreLog, "No manual sound specified");
                narrator->stop();
            }
        }
            break;

        case ClientCore::HELP_CLOSE:
        {
            LOG4CXX_INFO(clientcoreLog, "ClientCore::HELP_CLOSE received");
            state_->bHelpmode = false;
            state_->infoTimer = time(NULL);
            narrator->stop();
            narrator->play(_N("continuing"));
            navi_->process(COMMAND_LAST);
        }
            break;

        case ClientCore::ABOUT:
        {
            LOG4CXX_INFO(clientcoreLog, "ClientCore::ABOUT received");
            narrator->stop();
            player->pause();

            state_->bHelpmode = true;

            std::string about = clientcore_->getAboutSound();
            if (about != "")
            {
                // Split useragent into model/version
                std::string useragent = clientcore_->getUserAgent();
                std::string::size_type slashpos = useragent.find('/');
                if (slashpos != std::string::npos)
                {
                    std::string model = useragent.substr(0, slashpos);
                    std::string version = useragent.substr(slashpos + 1);
                    if (not model.empty() && not version.empty())
                    {
                        // narrate model
                        narrator->play(_N("model name"));

                        // narrate version
                        narrator->spell(version);
                    }
                }
                narrator->playFile(about);
                // Wait a while, then stop player so that it doesn't start when narrator finishes
                usleep(500000);
                player->stop();
            }
            else
            {
                LOG4CXX_ERROR(clientcoreLog, "No about sound specified");
                narrator->stop();
            }
        }
            break;

        case ClientCore::ABOUT_CLOSE:
        {
            LOG4CXX_INFO(clientcoreLog, "ClientCore::ABOUT_CLOSE received");
            state_->bHelpmode = false;
            state_->infoTimer = time(NULL);
            narrator->stop();
            narrator->play(_N("continuing"));
            navi_->process(COMMAND_LAST);
        }
            break;

        case ClientCore::SPEEDUP:
        {
            LOG4CXX_INFO(clientcoreLog, "ClientCore::SPEEDUP received");
            narrator->stop();
            player->pause();
            player->adjustTempo(+0.1);
            narrator->adjustTempo(+0.1);

            // Announce the new speed
            int newSpeed = (int) (player->getTempo() * 10.0 + 0.1) - 10;
            LOG4CXX_DEBUG(clientcoreLog, "New playback rate: " << player->getTempo() << "(" << newSpeed << ")");
            if (newSpeed == 0)
            {
                narrator->play(_N("normal speed"));
            }
            else if (newSpeed < 0)
            {
                narrator->play(_N("increasing speed to"));
                narrator->play(newSpeed);
            }
            else if (newSpeed > 0)
            {
                narrator->play(_N("increasing speed to"));
                narrator->play(_N("plus"));
                narrator->play(newSpeed);
            }

            settings->write<double>("playbackspeed", player->getTempo());
        }
            break;

        case ClientCore::SPEEDDOWN:
        {
            LOG4CXX_INFO(clientcoreLog, "ClientCore::SPEEDDOWN received");
            narrator->stop();
            player->pause();
            player->adjustTempo(-0.1);
            narrator->adjustTempo(-0.1);

            // Announce the new speed
            int newSpeed = (int) (player->getTempo() * 10.0 + 0.1) - 10;
            LOG4CXX_DEBUG(clientcoreLog, "New playback rate: " << player->getTempo() << "(" << newSpeed << ")");
            if (newSpeed == 0)
            {
                narrator->play(_N("normal speed"));

            }
            else if (newSpeed < 0)
            {
                narrator->play(_N("decreasing speed to"));
                narrator->play(newSpeed);
            }
            else if (newSpeed > 0)
            {
                narrator->play(_N("decreasing speed to"));
                narrator->play(_N("plus"));
                narrator->play(newSpeed);
            }

            settings->write<double>("playbackspeed", player->getTempo());
        }
            break;

        case ClientCore::SLEEP_OFF:
            clientcore_->setSleepTimerTimeLeft(-1);
            break;

        case ClientCore::SLEEP_15:
            clientcore_->setSleepTimerTimeLeft(15);
            break;
        case ClientCore::SLEEP_30:
            clientcore_->setSleepTimerTimeLeft(30);
            break;

        case ClientCore::SLEEP_60:
            clientcore_->setSleepTimerTimeLeft(60);
            break;

        case ClientCore::RETRY_LOGIN:
        {
            LOG4CXX_INFO(clientcoreLog, "ClientCore::RETRY_LOGIN received");
            narrator->stop();
            cq2::Command<INTERNAL_COMMAND> c(COMMAND_RETRY_LOGIN_FORCED);
            c();
        }
            break;

        case ClientCore::NARRATOR_CONTROL_STOP:
        {
            LOG4CXX_INFO(clientcoreLog, "ClientCore::NARRATOR_CONTROL_STOP received");
            narrator->stop();
            player->pause();
            // Disable narrator signals when finished
            narrator->setPushCommandFinished(false);
        }
            break;

        case ClientCore::NARRATOR_CONTROL_CONTINUE:
        {
            LOG4CXX_INFO(clientcoreLog, "ClientCore::NARRATOR_CONTROL_CONTINUE received");
            narrator->stop();
            player->resume();
            // Enable narrator signals when finished
            narrator->setPushCommandFinished(true);
        }
            break;

        default:
            LOG4CXX_INFO(clientcoreLog, "Not processing command here, sending command to navi");
            // Convert external ClientCore command to internal command. By re-mapping the command we maintain
            // functinality. However, it void be better to organize the commands into smaller groups, based
            // on the receiver of the command, and create separate handlers for each group.
            INTERNAL_COMMAND internalCommand = COMMAND_NONE;
            switch (command)
            {
            case ClientCore::DOWN:
                internalCommand = COMMAND_DOWN;
                break;
            case ClientCore::BOOKMARK:
                internalCommand = COMMAND_BOOKMARK;
                break;
            case ClientCore::CONTEXTMENU:
                internalCommand = COMMAND_OPEN_CONTEXTMENU;
                break;
            case ClientCore::LEFT:
                internalCommand = COMMAND_LEFT;
                break;
            case ClientCore::PAUSE:
                internalCommand = COMMAND_PAUSE;
                break;
            case ClientCore::BACK:
                internalCommand = COMMAND_BACK;
                break;
            case ClientCore::RIGHT:
                internalCommand = COMMAND_RIGHT;
                break;
            case ClientCore::UP:
                internalCommand = COMMAND_UP;
                break;
            case ClientCore::HOME:
                internalCommand = COMMAND_HOME;
                break;
            case ClientCore::BOOKINFO:
                internalCommand = COMMAND_OPEN_BOOKINFO;
                break;
            case ClientCore::GOTO_TIME:
                internalCommand = COMMAND_OPEN_MENU_GOTOTIMENODE;
                break;
            case ClientCore::GOTO_PERCENT:
                internalCommand = COMMAND_OPEN_MENU_GOTOPERCENTNODE;
                break;
            case ClientCore::GOTO_PAGE:
                internalCommand = COMMAND_OPEN_MENU_GOTOPAGENODE;
                break;
            }
            navi_->process(internalCommand);
            break;
        }
    }
};

struct Handle_InternalCommands: public cq2::Handler<INTERNAL_COMMAND>
{
    Handle_InternalCommands(Navi* navi) :
            navi_(navi)
    {
    }

private:
    Navi* navi_;

    void handle(INTERNAL_COMMAND command)
    {
        navi_->process(command);
    }
};

struct Handle_NotifyCommands: public cq2::Handler<NOTIFY_COMMAND>
{
    Handle_NotifyCommands(ClientCore* clientcore, ClientCoreState* state) :
            clientcore_(clientcore), state_(state)
    {
    }

private:
    ClientCore* clientcore_;
    ClientCoreState* state_;

    void handle(NOTIFY_COMMAND command)
    {
        switch (command)
        {
        case NOTIFY_SLEEP_TIMEOUT:
        {
            LOG4CXX_DEBUG(clientcoreLog, "NOTIFY_SLEEP_TIMEOUT received");
            clientcore_->sleepTimeout_signal();
        }
            break;
        case NOTIFY_LOGIN_OK:
        {
            LOG4CXX_DEBUG(clientcoreLog, "NOTIFY_LOGIN_OK received");
            state_->retryLogin = false;
            clientcore_->loginResult_signal(true);
        }
            break;
        case NOTIFY_LOGIN_FAIL:
        {
            LOG4CXX_WARN(clientcoreLog, "NOTIFY_LOGIN_FAIL received");
            state_->retryLogin = true;
            clientcore_->loginResult_signal(false);
        }
            break;
        case NOTIFY_INVALID_AUTH:
        {
            LOG4CXX_DEBUG(clientcoreLog, "NOTIFY_INVALID_AUTH received");
            state_->retryLogin = true;
            clientcore_->invalidAuth_signal();
        }
            break;
        }
    }
};

struct Handle_JumpToUri: public cq2::Handler<JumpCommand<std::string> >
{
    Handle_JumpToUri(Navi* navi) :
            navi_(navi)
    {
    }

private:
    Navi* navi_;

    void handle(JumpCommand<std::string> jump)
    {
        LOG4CXX_DEBUG(clientcoreLog, "JumpCommand<std::string> received with target '" << jump.target_ << "'");
        navi_->selectNodeByUri(jump.target_);
    }
};

struct Handle_ErrorMessage: public cq2::Handler<ErrorMessage>
{
    Handle_ErrorMessage(ClientCore* clientcore) :
            clientcore_(clientcore)
    {
    }

private:
    ClientCore* clientcore_;

    void handle(ErrorMessage error)
    {
        LOG4CXX_ERROR(clientcoreLog, "ErrorMessage received");
        clientcore_->errorMessage_signal(error);
    }
};

struct Handle_BookData: public cq2::Handler<ClientCore::BookData>
{
    Handle_BookData(ClientCore* clientcore) :
            clientcore_(clientcore)
    {
    }

private:
    ClientCore* clientcore_;

    void handle(ClientCore::BookData bookData)
    {
        LOG4CXX_DEBUG(clientcoreLog, "ClientCore::BookData received");
        clientcore_->bookData_signal(bookData);
    }
};

struct Handle_BookPosition: public cq2::Handler<BookPositionInfo>
{
    Handle_BookPosition(ClientCore* clientcore) :
            clientcore_(clientcore)
    {
    }

private:
    ClientCore* clientcore_;

    void handle(BookPositionInfo position)
    {
        LOG4CXX_DEBUG(clientcoreLog, "BookPositionInfo received");
        clientcore_->bookPosition_signal(position);
    }
};

struct Handle_BookPage: public cq2::Handler<BookPageInfo>
{
    Handle_BookPage(ClientCore* clientcore) :
            clientcore_(clientcore)
    {
    }

private:
    ClientCore* clientcore_;

    void handle(BookPageInfo page)
    {
        LOG4CXX_DEBUG(clientcoreLog, "BookPageInfo received");
        clientcore_->bookPage_signal(page);
    }
};

struct Handle_BookSection: public cq2::Handler<BookSectionInfo>
{
    Handle_BookSection(ClientCore* clientcore) :
            clientcore_(clientcore)
    {
    }

private:
    ClientCore* clientcore_;

    void handle(BookSectionInfo section)
    {
        LOG4CXX_DEBUG(clientcoreLog, "BookSectionInfo received");
        clientcore_->bookSection_signal(section);
    }
};

struct Handle_DaisyNaviLevel: public cq2::Handler<DaisyNaviLevel>
{
    Handle_DaisyNaviLevel(ClientCore* clientcore) :
            clientcore_(clientcore)
    {
    }

private:
    ClientCore* clientcore_;

    void handle(DaisyNaviLevel level)
    {
        LOG4CXX_DEBUG(clientcoreLog, "DaisyNaviLevel received");
        clientcore_->daisyNaviLevel_signal(level);
    }
};

struct Handle_NaviList: public cq2::Handler<NaviList>
{
    Handle_NaviList(ClientCore* clientcore) :
            clientcore_(clientcore)
    {
    }

private:
    ClientCore* clientcore_;

    void handle(NaviList list)
    {
        LOG4CXX_DEBUG(clientcoreLog, "NaviList received");
        clientcore_->naviList_signal(list);
    }
};

struct Handle_NaviListItem: public cq2::Handler<NaviListItem>
{
    Handle_NaviListItem(ClientCore* clientcore) :
            clientcore_(clientcore)
    {
    }

private:
    ClientCore* clientcore_;

    void handle(NaviListItem item)
    {
        LOG4CXX_DEBUG(clientcoreLog, "NaviListItem received");
        clientcore_->naviListItem_signal(item);
    }
};

// command handlers end

void *ClientCore::clientcore_thread(void *ctx)
{
    ClientCore* ctxptr = (ClientCore*) ctx;

    // say welcome
    Narrator::Instance()->play(_N("welcome"));
    usleep(500000);
    while (Narrator::Instance()->isSpeaking())
    {
        usleep(20000);
    }

    // Default to running
    bool running = true;

    // Get Narrator and Player instances
    Narrator *narrator = Narrator::Instance();
    Player *player = Player::Instance();

    // Initialize the root node to use in NaviEngine
    std::string userAgent = ctxptr->getUserAgent();
    RootNode *rootNode = new RootNode(userAgent);

    Navi *navi = new Navi(ctxptr);
    navi->openMenu(rootNode, true);
    if (not navi->good())
    {
        // Failed to open the root node.
        LOG4CXX_ERROR(clientcoreLog, "failed to root node");
        pthread_mutex_lock(&ctxptr->clientcoreMutex);
        running = ctxptr->clientcoreRunning = false;
        pthread_mutex_unlock(&ctxptr->clientcoreMutex);
    }

    ClientCoreState state;

    // Command handlers
    Handle_ClientCoreCommands commandHandler(navi, ctxptr, &state);
    commandHandler.listen();

    Handle_InternalCommands internalHandler(navi);
    internalHandler.listen();

    Handle_NotifyCommands notifyHandler(ctxptr, &state);
    notifyHandler.listen();

    Handle_JumpToUri jumpUriHandler(navi);
    jumpUriHandler.listen();

    Handle_ErrorMessage errorHandler(ctxptr);
    errorHandler.listen();

    Handle_BookData bookDataHandler(ctxptr);
    bookDataHandler.listen();

    Handle_BookPosition bookPosHandler(ctxptr);
    bookPosHandler.listen();

    Handle_BookPage bookPageHandler(ctxptr);
    bookPageHandler.listen();

    Handle_BookSection bookSectHandler(ctxptr);
    bookSectHandler.listen();

    Handle_DaisyNaviLevel daisyLevelHandler(ctxptr);
    daisyLevelHandler.listen();

    Handle_NaviList naviListHandler(ctxptr);
    naviListHandler.listen();

    Handle_NaviListItem naviItemHandler(ctxptr);
    naviItemHandler.listen();

    // Command loop

    while (running)
    {
        // process SleepTimer
        int sleepTimerTimeLeft = ctxptr->getSleepTimerTimeLeft();
        SleepTimerStates sleepTimerState = ctxptr->getSleepTimerState();
        switch (sleepTimerState)
        {
        case SLEEP_TIMER_ON:
            // Inform user that SleepTimer is about to run out
            if (sleepTimerTimeLeft < SLEEP_TIMER_NEAR_TIMEOUT)
            {
                LOG4CXX_INFO(clientcoreLog, "SleepTimer near timeout");
                ctxptr->setSleepTimerState(SLEEP_TIMER_NEAR_TIMEOUT);
                cq2::Command<NOTIFY_COMMAND> notify(NOTIFY_SLEEP_NEAR_TIMEOUT);
                notify();
                player->pause();
                narrator->stop();
                narrator->play(_N("Sleep timer is running out. Application will close soon."));
                narrator->play(_N("Turn off automatic shutdown by pressing an application button."));
            }
            break;
        case SLEEP_TIMER_NEAR_TIMEOUT:
            // Inform user that SleepTimer has run out
            if (sleepTimerTimeLeft < SLEEP_TIMER_TIMED_OUT)
            {
                LOG4CXX_INFO(clientcoreLog, "SleepTimer timeout occurred");
                ctxptr->setSleepTimerState(SLEEP_TIMER_TIMED_OUT);
                cq2::Command<NOTIFY_COMMAND> notify(NOTIFY_SLEEP_TIMEOUT);
                notify();
            }
            break;
        case SLEEP_TIMER_TIMED_OUT:
            // Exit when SleepTimer has run out
            if (not state.sleepTimerTimeout)
            {
                state.sleepTimerTimeout = true;
                cq2::Command<ClientCore::COMMAND> exit(ClientCore::EXIT);
                exit();
            }
            break;
        }

        if (cq2::Dispatcher::instance().dispatchCommand())
        {
            state.commandQueueEmpty = false;
            continue;
        }

        if (not state.commandQueueEmpty)
        {
            // the command queue is empty
            state.commandQueueEmpty = true;
            LOG4CXX_DEBUG(clientcoreLog, "Command queue is empty, emitting signal");
            ctxptr->queueEmpty_signal();
        }

        if (time(NULL) > state.infoTimer && !state.bHelpmode)
        {
            LOG4CXX_DEBUG(clientcoreLog, "infoTimer timeout occurred");
            state.infoTimer = time(NULL) + INFO_TIMEOUT;

            // send info events unless sleep timer is about to run out
            if (sleepTimerState != SLEEP_TIMER_NEAR_TIMEOUT)
            {
                // retryLogin
                if (state.retryLogin)
                {
                    LOG4CXX_DEBUG(clientcoreLog, "SEND RETRY_LOGING command");
                    cq2::Command<INTERNAL_COMMAND> c(COMMAND_RETRY_LOGIN);
                    c();
                }
                else
                {
                    LOG4CXX_DEBUG(clientcoreLog, "SEND INFO command");
                    cq2::Command<INTERNAL_COMMAND> c(COMMAND_INFO);
                    c();
                }
                ctxptr->infoTimeout_signal();
            }
        }

        usleep(20000);

        // While speaking or playing, push infoTimer forward
        if (narrator->isSpeaking() || player->isPlaying())
            state.infoTimer = time(NULL) + INFO_TIMEOUT;

        pthread_mutex_lock(&ctxptr->clientcoreMutex);
        running = ctxptr->clientcoreRunning;
        pthread_mutex_unlock(&ctxptr->clientcoreMutex);

        if (!running)
            break;
    }

    narrator->stop();
    player->stop();

    narrator->play(_N("shutting down"));

    LOG4CXX_DEBUG(clientcoreLog, "Deleting navi");
    delete navi;
    LOG4CXX_DEBUG(clientcoreLog, "Deleting player");
    delete player;

    // Wait for narrator to finish before deleting it
    usleep(500000);
    while (Narrator::Instance()->isSpeaking())
    {
        usleep(20000);
    }

    LOG4CXX_DEBUG(clientcoreLog, "Deleting narrator");
    delete narrator;

    return NULL;
}
