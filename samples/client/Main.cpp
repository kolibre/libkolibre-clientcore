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

#include "Input.h"

#include <ClientCore.h>

#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <cstring>
#include <glib.h>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

// create a root logger
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("kolibre"));

// create logger which will become a child to logger kolibre.sampleclient
log4cxx::LoggerPtr sampleClientMainLog(log4cxx::Logger::getLogger("kolibre.sampleclient.main"));

// exit values:     0   -> NORMAL QUIT
//                  2   -> SIGINT
//                  3   -> SIGQUIT
//                  15  -> SIGTERM
//                  100 -> SLEEPTIMER TIMEOUT
//                  101 -> LOGIN FAILED
int exitValue = 0;
bool exitSignal = false;
void handleSignal(int sig);
void onSleepTimeout();
void onInvalidAuth();
void exitApplication();


char* applicationPath;
void usage()
{
    printf("usage: %s [OPTIONS]\n", applicationPath);
    printf("OPTIONS\n");
    printf("  -s <url> \t\turl to Daisy Online service\n");
    printf("  -u <username> \tusername for service, must be specified along with -s option\n");
    printf("  -p <password> \tpassword for service, must be specified along with -s option\n");
    printf("  -r \t\t\tremember password for specified service\n");
    printf("  -m <path> \t\tpath to local media\n");
    printf("  -l <lang> \t\tlanguage to use, options: sv, fi, en, ru [default: en]\n");
    printf("  -i <path> \t\tpath to client configuration\n");
    printf("  -c <path> \t\tpath to log configuration\n");
    printf("  -d <path> \t\tpath to input device\n");
    printf("  -a <useragent> \tstring to use as UserAgent\n");
    printf("  -h \t\t\tshow this message\n");
    printf("\n");
    printf("Note! You must specify either a Daisy Online service, a local media path\n");
    printf("or client configuration path.\n");
    printf("   e.g. %s -s http://daisyonline.com/service -u username -p password\n", applicationPath);
    printf("        %s -m /home/user/Media\n", applicationPath);
    printf("        %s -i /home/user/settings.ini\n", applicationPath);
}

int main(int argc, char **argv)
{
    applicationPath = argv[0];
    if (argc < 2)
    {
        usage();
        return 1;
    }

    signal(SIGINT, handleSignal);
    signal(SIGQUIT, handleSignal);
    signal(SIGTERM, handleSignal);

    // variables for storing user arguments
    std::string serviceUrl, username, password = "";
    bool rememberPassword = false;
    std::string mediaPath = "";
    std::string language = "en"; // default to English
    std::string inputDev = "";
    std::string useragent = "KolibreSampleClient/1.1.0";
    char *settingsPath = NULL;
    char *logConf = NULL;

    // parse user arguments
    int opt;
    while ((opt = getopt(argc, argv, "s:u:p:rm:l:i:c:d:a:h")) != -1)
    {
        switch (opt)
        {
        case 's':
        {
            serviceUrl = optarg;
            mediaPath = "";
            settingsPath = NULL;
            break;
        }
        case 'u':
        {
            username = optarg;
            break;
        }
        case 'p':
        {
            password = optarg;
            break;
        }
        case 'r':
            rememberPassword = true;
            break;
        case 'm':
        {
            serviceUrl = "";
            mediaPath = optarg;
            settingsPath = NULL;
            break;
        }
        case 'l':
        {
            if (strcmp(optarg, "en") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "sv") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "fi") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "ru") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "sq") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "ar") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "hy") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "af") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "bs") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "cy") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "hu") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "vi") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "el") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "da") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "id") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "is") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "es") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "it") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "ca") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "zh") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "ko") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "ht") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "la") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "lv") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "mk") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "de") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "nl") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "no") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "pl") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "pt") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "ro") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "sr") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "sk") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "sw") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "th") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "ta") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "tr") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "fr") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "hi") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "hr") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "cs") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "eo") == 0)
            {
                language = optarg;
            }
            else if (strcmp(optarg, "ja") == 0)
            {
                language = optarg;
            }

//            language = "en";

            break;
        }
        case 'i':
        {
            serviceUrl = "";
            mediaPath = "";
            settingsPath = optarg;
            break;
        }
        case 'c':
        {
            logConf = optarg;
            break;
        }
        case 'd':
        {
            inputDev = optarg;
            break;
        }
        case 'a':
        {
            useragent = optarg;
            break;
        }
        case 'h':
        {
            usage();
            return 0;
        }
        default:
            printf("Unknown option: %c\n", opt);
            break;
        }
    }

    // check user arguments
    if (serviceUrl.empty() && mediaPath.empty() && settingsPath == NULL)
    {
        usage();
        return 1;
    }
    else if (not serviceUrl.empty())
    {
        if (username.empty() || password.empty())
        {
            usage();
            return 1;
        }
    }

    // Setup logging
    try
    {
        if (logConf != NULL)
        {
            // set up a user defined configuration
            log4cxx::PropertyConfigurator::configure(logConf);
        }
        else
        {
            // set up a simple configuration that logs on the console
            log4cxx::BasicConfigurator::configure();
        }
    }
    catch (log4cxx::helpers::Exception&)
    {
        printf("Configuration of logger failed\n");
        return -1;
    }

    // Set language
    ClientCore::setLanguage(language);

    ClientCore *clientcore = new ClientCore(useragent);

    // add service or path or parse settings file
    if (not serviceUrl.empty())
    {
        clientcore->addDaisyOnlineService("main", serviceUrl, username, password, rememberPassword);
    }
    else if (not mediaPath.empty())
    {
        clientcore->addFileSystemPath("main", mediaPath);
    }
    else if (settingsPath != NULL)
    {
        GKeyFile *keyFile;
        GKeyFileFlags keyFileFlags = G_KEY_FILE_NONE;
        GError *error;

        // create a new GKeyFile object
        keyFile = g_key_file_new();

        // load data from file
        if (not g_key_file_load_from_file(keyFile, settingsPath, keyFileFlags, &error))
        {
            LOG4CXX_ERROR(sampleClientMainLog, "error while loading file '" << settingsPath << "':" << error->message);
            g_key_file_free(keyFile);
            delete clientcore;
            return -1;
        }

        // get all groups in file
        gchar **groups = NULL;
        gsize length;
        groups = g_key_file_get_groups(keyFile, &length);

        // loop through each group and search for matches
        for (unsigned long i = 0; i < length; i++)
        {
            // if group name contains 'DaisyOnlineService'
            if (strstr(groups[i], "DaisyOnlineService") != NULL)
            {
                LOG4CXX_INFO(sampleClientMainLog, "Found DaisyOnline service group '" << groups[i] << "'");
                bool missingKey = false;
                gchar *name, *url, *username, *password = NULL;

                // get key name
                name = g_key_file_get_string(keyFile, groups[i], "NAME", NULL);
                if (name == NULL)
                {
                    LOG4CXX_WARN(sampleClientMainLog, "Group '" << groups[i] << "' does not have key 'NAME'");
                    missingKey = true;
                }

                // get key url
                url = g_key_file_get_string(keyFile, groups[i], "URL", NULL);
                if (url == NULL)
                {
                    LOG4CXX_WARN(sampleClientMainLog, "Group '" << groups[i] << "' does not have key 'URL'");
                    missingKey = true;
                }

                // get key username
                username = g_key_file_get_string(keyFile, groups[i], "USERNAME", NULL);
                if (username == NULL)
                {
                    LOG4CXX_WARN(sampleClientMainLog, "Group '" << groups[i] << "' does not have key 'USERNAME'");
                    missingKey = true;
                }

                // get key password
                password = g_key_file_get_string(keyFile, groups[i], "PASSWORD", NULL);
                if (password == NULL)
                {
                    LOG4CXX_WARN(sampleClientMainLog, "Group '" << groups[i] << "' does not have key 'PASSWORD'");
                    missingKey = true;
                }

                if (not missingKey)
                {
                    LOG4CXX_INFO(sampleClientMainLog, "Adding '" << name << "' as a DaisyOnlineService");
                    clientcore->addDaisyOnlineService(name, url, username, password);
                }

            }
            // if group name contains 'FileSystemPath'
            else if (strstr(groups[i], "FileSystemPath") != NULL)
            {
                LOG4CXX_INFO(sampleClientMainLog, "Found file system path group '" << groups[i] << "'");
                bool missingKey = false;
                gchar *name, *path = NULL;

                // get key name
                name = g_key_file_get_string(keyFile, groups[i], "NAME", NULL);
                if (name == NULL)
                {
                    LOG4CXX_WARN(sampleClientMainLog, "Group '" << groups[i] << "' does not have key 'NAME'");
                    missingKey = true;
                }

                // get key path
                path = g_key_file_get_string(keyFile, groups[i], "PATH", NULL);
                if (path == NULL)
                {
                    LOG4CXX_WARN(sampleClientMainLog, "Group '" << groups[i] << "' does not have key 'PATH'");
                    missingKey = true;
                }

                if (not missingKey)
                {
                    LOG4CXX_INFO(sampleClientMainLog, "Adding '" << name << "' as a FileSystemPath");
                    clientcore->addFileSystemPath(name, path);
                }
            }
            // if group name contains 'MP3Path'
            else if (strstr(groups[i], "MP3Path") != NULL)
            {
                LOG4CXX_INFO(sampleClientMainLog, "Found mp3 path group '" << groups[i] << "'");
                bool missingKey = false;
                gchar *name, *path = NULL;

                // get key name
                name = g_key_file_get_string(keyFile, groups[i], "NAME", NULL);
                if (name == NULL)
                {
                    LOG4CXX_WARN(sampleClientMainLog, "Group '" << groups[i] << "' does not have key 'NAME'");
                    missingKey = true;
                }

                // get key path
                path = g_key_file_get_string(keyFile, groups[i], "PATH", NULL);
                if (path == NULL)
                {
                    LOG4CXX_WARN(sampleClientMainLog, "Group '" << groups[i] << "' does not have key 'PATH'");
                    missingKey = true;
                }

                if (not missingKey)
                {
                    LOG4CXX_INFO(sampleClientMainLog, "Adding '" << name << "' as a FileSystemPath");
                    clientcore->addMP3Path(name, path);
                }
            }
        }

        // free allocated memory
        g_strfreev(groups);
        g_key_file_free(keyFile);
    }

    // Connect slots to signals
    clientcore->sleepTimeout_signal.connect(&onSleepTimeout);
    clientcore->invalidAuth_signal.connect(&onInvalidAuth);
    Input *input = Input::Instance();
    input->keyPressed_signal.connect(boost::bind(&ClientCore::pushCommand, clientcore, _1));
    if(inputDev.compare("") != 0)
        input->set_input_device(inputDev);

    // start client and wait for it to exit
    clientcore->start();
    while (clientcore->isRunning())
        usleep(100000);

    LOG4CXX_DEBUG(sampleClientMainLog, "Deleting clientcore");
    delete clientcore;

    LOG4CXX_DEBUG(sampleClientMainLog, "Deleting input");
    delete input;

    LOG4CXX_INFO(sampleClientMainLog, "Exiting application with value " << exitValue);
    return exitValue;
}

// Handle boost signals
void onSleepTimeout()
{
    exitValue = 100;
}

// Handle the different unix signals we might receive
void handleSignal(int sig)
{
    if (!exitSignal)
    {
        exitSignal = true;
        exitValue = sig;
        std::string signal;
        switch (sig)
        {
            case 2:
                signal = "SIGINT";
                break;
            case 3:
                signal = "SIGQUIT";
                break;
            case 15:
                signal = "SIGTERM";
                break;
            default:
                signal = "UNKNOWN";
                break;
        }
        LOG4CXX_INFO(sampleClientMainLog, "Caught signal '" << signal << "' (" << sig << "), exiting application");
        exitApplication();
    }
}

void onInvalidAuth()
{
    if (!exitSignal)
    {
        exitSignal = true;
        exitValue = 101;

        LOG4CXX_INFO(sampleClientMainLog, "Login failed, exiting application");
        exitApplication();
    }
}

void exitApplication()
{
    Input *input = Input::Instance();
    input->keyPressed_signal(ClientCore::EXIT);
    sleep(1);
}
