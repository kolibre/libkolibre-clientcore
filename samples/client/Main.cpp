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

// exit values:     0   -> NORMAL QUIT
//                  2   -> SIGINT
//                  3   -> SIGQUIT
//                  15  -> SIGTERM
//                  100 -> SLEEPTIMER TIMEOUT
int exitValue = 0;
bool exitSignal = false;
void handleSignal(int sig);
void onSleepTimeout();

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("usage: %s <service_url> <username> <password> [OPTIONS]\n", argv[0]);
        printf(" OPTIONS\n");
        printf("       -r Remember password\n");
        printf("       -l language options: fi, se, en [default: sv]\n");
        printf("       -c log configuration file\n");
        return 1;
    }
    signal(SIGINT, handleSignal);
    signal(SIGQUIT, handleSignal);
    signal(SIGTERM, handleSignal);

    // store required arguments before parsing optional
    std::string service_url, username, password;
    service_url = argv[1];
    username = argv[2];
    password = argv[3];

    bool rememberPassword = false;

    // Initiate logging
    char *logConf = NULL;

    // Initiate lanuage variable with default swedish
    std::string language = "sv";

    // Handle option flags
    int opt;
    while ((opt = getopt(argc, argv, "rl:c:")) != -1)
    {
        switch (opt)
        {
        case 'r':
            rememberPassword = true;
            break;
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
        case 'c':
        {
            logConf = optarg;
            break;
        }
        default:
            printf("Unknown option: %c", opt);
            break;
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

    ClientCore *clientcore = new ClientCore(service_url, "KolibreSampleClient/0.0.1");

    clientcore->setUsername(username);
    clientcore->setPassword(password, rememberPassword);

    // Connect slots to signals
    clientcore->sleepTimeout_signal.connect(&onSleepTimeout);
    Input *input = Input::Instance();
    input->keyPressed_signal.connect(boost::bind(&ClientCore::pushCommand, clientcore, _1));

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

// Handle the different unix signals we might recieve
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
        Input *input = Input::Instance();
        input->keyPressed_signal(ClientCore::EXIT);
        sleep(1);
    }
}
