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
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

// create a root logger
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("kolibre"));

// create logger which will become a child to logger kolibre.sampleclient
log4cxx::LoggerPtr sampleClientMainLog(log4cxx::Logger::getLogger("kolibre.sampleclient.main"));

// exit values:     0 -> SIGINT, SIGQUIT, SIGTERM
//                  1 -> SLEEPTIMER
int exitValue = 0;
bool exitSignal = false;
void handleSignal(int sig);
void onSleepTimeout();

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
    printf("  -l <lang> \t\tlanguage to use, options: sv, fi, en [default: sv]\n");
    printf("  -i <path> \t\tpath to client configuration\n");
    printf("  -c <path> \t\tpath to log configuration\n");
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
    std::string language = "sv"; // default to Swedish
    char *settingsPath = NULL;
    char *logConf = NULL;

    // parse user arguments
    int opt;
    while ((opt = getopt(argc, argv, "s:u:p:rm:l:i:c:h")) != -1)
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
    else if (not mediaPath.empty())
    {
        printf("error: Support not yet implemented\n");
        return 1;
    }
    else if (settingsPath != NULL)
    {
        printf("error: Support not yet implemented\n");
        return 1;
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

    ClientCore *clientcore = new ClientCore("KolibreSampleClient/0.0.1");
    clientcore->addDaisyOnlineService("main", serviceUrl, username, password, rememberPassword);

    // Connect slots to signals
    clientcore->sleepTimeout_signal.connect(&onSleepTimeout);
    Input *input = Input::Instance();
    input->keyPressed_signal.connect(boost::bind(&ClientCore::pushCommand, clientcore, _1));

    // start client and wait for it to exit
    clientcore->start();
    while (clientcore->isRunning())
        usleep(100000);

    LOG4CXX_DEBUG(sampleClientMainLog, "Deleting clientcore");
    delete clientcore;

    LOG4CXX_DEBUG(sampleClientMainLog, "Deleting input");
    delete input;

    return exitValue;
}

// Handle boost signals
void onSleepTimeout()
{
    exitValue = -1;
}

// Handle the different unix signals we might receive
void handleSignal(int sig)
{
    if (!exitSignal)
    {
        exitSignal = true;
        LOG4CXX_INFO(sampleClientMainLog, "Caught signal (" << sig << "), exiting");
        Input *input = Input::Instance();
        input->keyPressed_signal(ClientCore::EXIT);
        sleep(1);
    }
}
