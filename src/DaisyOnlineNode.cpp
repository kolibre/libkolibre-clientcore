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

#include "DaisyOnlineNode.h"
#include "DaisyOnlineBookNode.h"
#include "ClientCore.h"
#include "Defines.h"
#include "config.h"
#include "version.h"
#include "Commands/NotifyCommands.h"
#include "Commands/InternalCommands.h"
#include "CommandQueue2/CommandQueue.h"
#include "MediaSourceManager.h"
#include "Utils.h"

#include <DataStreamHandler.h>
#include <Narrator.h>
#include <NaviEngine.h>

#include <time.h>
#include <iostream>
#include <libintl.h>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr onlineNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.daisyonlinenode"));

using namespace naviengine;

DaisyOnlineNode::DaisyOnlineNode(const std::string name, const std::string uri, const std::string username, const std::string password, std::string useragent) :
        good_(true), currentChild_(0)
{
    LOG4CXX_TRACE(onlineNodeLog, "Constructor");
    name_ = "DaisyOnline_" + name;
    serviceName_ = name;
    serviceUri_ = uri;
    username_ = username;
    password_ = password;
    previousUsername_ = "";
    previousPassword_ = "";
    lastUpdate_ = -1;
    lastError_ = (DaisyOnlineNode::errorType)-1;
    lastLogOnAttempt_ = (DaisyOnlineNode::errorType)-1;
    loggedIn_ = false;
    serviceUpdated_ = false;

    if (useragent.length() == 0)
        useragent = string(VERSION_PACKAGE_NAME) + "/" + VERSION_PACKAGE_VERSION;

    NaviList navilist;
    navilist.name_ = _("Logging in");
    navilist.info_ = _("Connecting to content provider, please wait");
    cq2::Command<NaviList> naviList(navilist);
    naviList();

    pDOHandler = new DaisyOnlineHandler(uri, useragent);
    // Check if initialization failed
    if (not pDOHandler->good())
    {
        good_ = false;
        return;
    }

    language_ = "i-unknown";
    manufacturer_ = "Kolibre";
    model_ = string(VERSION_PACKAGE_NAME);
    version_ = string(VERSION_PACKAGE_VERSION);
    serialNumber_ = "";

    // connect slots to signals
    sessionInit_signal.connect(boost::bind(&DaisyOnlineNode::onSessionInit, this));
    issueContent_signal.connect(boost::bind(&DaisyOnlineNode::onIssueContent, this));
}

DaisyOnlineNode::~DaisyOnlineNode()
{
    LOG4CXX_TRACE(onlineNodeLog, "Destructor");
    delete pDOHandler;
}

void DaisyOnlineNode::setManufacturer(const std::string &manufacturer)
{
    manufacturer_ = manufacturer;
}

void DaisyOnlineNode::setModel(const std::string &model)
{
    model_ = model;
}

void DaisyOnlineNode::setSerialNumber(const std::string &serialNumber)
{
    serialNumber_ = serialNumber;
}

void DaisyOnlineNode::setVersion(const std::string &version)
{
    version_ = version;
}

void DaisyOnlineNode::setLanguage(const std::string &language)
{
    language_ = language;
}

// NaviEngine functions

bool DaisyOnlineNode::next(NaviEngine& navi)
{
    bool ret = MenuNode::next(navi);
    currentChild_ = navi.getCurrentChoice();
    announceSelection();
    return ret;
}

bool DaisyOnlineNode::prev(NaviEngine& navi)
{
    bool ret = MenuNode::prev(navi);
    currentChild_ = navi.getCurrentChoice();
    announceSelection();
    return ret;
}

bool DaisyOnlineNode::menu(NaviEngine& navi)
{
    return navi.openMenu(navi.buildContextMenu());
}

bool DaisyOnlineNode::up(NaviEngine& navi)
{
    loggedIn_ = false;
    serviceUpdated_ = false;
    bool ret = MenuNode::up(navi);
    return ret;
}

bool DaisyOnlineNode::narrateName()
{
    const bool isSelfNarrated = true;
    Narrator::Instance()->play(_N("online service"));
    return isSelfNarrated;
}

bool DaisyOnlineNode::narrateInfo()
{
    const bool isSelfNarrated = true;
    Narrator::Instance()->play(_N("choose option using left and right arrows, open using play button"));
    Narrator::Instance()->playLongpause();
    announceSelection();
    return isSelfNarrated;
}

bool DaisyOnlineNode::onNarrate()
{
    const bool isSelfNarrated = true;
    return isSelfNarrated;
}

bool DaisyOnlineNode::onRender()
{
    const bool isSelfRendered = true;
    return isSelfRendered;
}

DaisyOnlineNode::errorType DaisyOnlineNode::getLastError()
{
    return lastError_;
}

std::string DaisyOnlineNode::getErrorMessage()
{
    return pDOHandler->getStatusMessage();
}

bool DaisyOnlineNode::good()
{
    return good_;
}

void DaisyOnlineNode::onSessionInit()
{
    LOG4CXX_DEBUG(onlineNodeLog, "sessionInit signal triggered");

    // Don't try session initialization if username or password hasn't changed ...
    int index = MediaSourceManager::Instance()->getDaisyOnlineServiceIndex(serviceName_);
    if (index >= 0)
    {
        username_ = MediaSourceManager::Instance()->getDOSusername(0);
        password_ = MediaSourceManager::Instance()->getDOSpassword(0);
    }
    else
    {
        username_ = "";
        password_ = "";
    }

    if (previousUsername_ == username_ && previousPassword_ == password_)
    {
        // ... and last logon attempt failed due to incorrect username or password
        if (lastLogOnAttempt_ == USERNAME_PASSWORD_ERROR)
        {
            LOG4CXX_WARN(onlineNodeLog, "aborting session initialization since nothing has changed since last attempt");
            return;
        }
    }

    // Notify user that logon failed if username is not set
    if (username_.empty())
    {
        cq2::Command<NOTIFY_COMMAND> notify(NOTIFY_LOGIN_FAIL);
        notify();
        return;
    }

    previousUsername_ = username_;
    previousPassword_ = password_;
    lastLogOnAttempt_ = sessionInit();

    // Log and inform user if session initialization failed
    if (lastLogOnAttempt_ != OK)
    {
        announceResult(lastLogOnAttempt_);
        LOG4CXX_ERROR(onlineNodeLog, "Session initialization failed");
        return;
    }

    cq2::Command<NOTIFY_COMMAND> notify(NOTIFY_LOGIN_OK);
    notify();

    // wait for narrator to finish speaking
    usleep(500000);
    while (Narrator::Instance()->isSpeaking())
    {
        usleep(20000);
    };

    // emit signal to issue new content
    issueContent_signal();
}

void DaisyOnlineNode::onIssueContent()
{
    // automatically issue new content
    errorType autoResult = OK;
    autoResult = autoIssueContentList();
    if (autoResult != OK)
    {
        // announce result for auto issue attempt
        announceResult(autoResult);

        // push COMMAND_HOME to command queue and return to the very beginning
        cq2::Command<ClientCore::COMMAND> c(ClientCore::HOME);
        c();
    }

    // send command to get content list
    cq2::Command<INTERNAL_COMMAND> c(COMMAND_DO_GETCONTENTLIST);
    c();
}

DaisyOnlineNode::errorType DaisyOnlineNode::sessionInit()
{
    good_ = false;
    loggedIn_ = false;
    // we must set lastUpdate_ here to block queued update requests
    lastUpdate_ = time(NULL);
    LOG4CXX_INFO(onlineNodeLog, "Trying to establish a Daisy Online session");

    // logOn
    bool logOnResult = pDOHandler->logOn(username_, password_);
    if (!pDOHandler->good())
    {
        LOG4CXX_ERROR(onlineNodeLog, "Error occurred when invoking logOn, " << pDOHandler->getStatus() << " '" << pDOHandler->getStatusMessage() << "'");
        return faultHandler(pDOHandler->getStatus());
    }
    if (!logOnResult)
    {
        LOG4CXX_WARN(onlineNodeLog, "logOn failed, service return false, please check check username and password");
        errorstring_ = "wrong username or password";
        lastError_ = USERNAME_PASSWORD_ERROR;
        // if logOn failed we allow new update requests
        lastUpdate_ = -1;
        return lastError_;
    }

    // getServiceAttributes
    kdo::ServiceAttributes* serviceAttributes;
    serviceAttributes = pDOHandler->getServiceAttributes();
    if (!pDOHandler->good())
    {
        LOG4CXX_ERROR(onlineNodeLog, "Error occurred when invoking getServiceAttributes, " << pDOHandler->getStatus() << " '" << pDOHandler->getStatusMessage() << "'");
        return faultHandler(pDOHandler->getStatus());
    }
    if (serviceAttributes == NULL)
    {
        LOG4CXX_ERROR(onlineNodeLog, "getServiceAttributes failed, returned serviceAttributes is NULL");
        errorstring_ = "error getting service attributes";
        lastError_ = SERVICE_ERROR;
        return lastError_;
    }

    // build readingSystemAttributes object
    kdo::ReadingSystemAttributes readingSystemAttributes;
    readingSystemAttributes.setManufacturer(manufacturer_);
    readingSystemAttributes.setModel(model_);
    readingSystemAttributes.setSerialNumber(serialNumber_);
    readingSystemAttributes.setVersion(version_);
    readingSystemAttributes.setPreferredUILanguage(language_);
    readingSystemAttributes.addContentFormat("Daisy 2.02");
    readingSystemAttributes.addMimeType("audio/ogg");

    // setReadingsystemAttributes
    bool setReadingSystemAttributesResult = pDOHandler->setReadingSystemAttributes(readingSystemAttributes);
    if (!pDOHandler->good())
    {
        LOG4CXX_ERROR(onlineNodeLog, "Error occurred when invoking setReadingSystemAttributes, " << pDOHandler->getStatus() << " '" << pDOHandler->getStatusMessage() << "'");
        return faultHandler(pDOHandler->getStatus());
    }
    if (!setReadingSystemAttributesResult)
    {
        LOG4CXX_WARN(onlineNodeLog, "setReadingSystemAttributes failed, service return false");
        errorstring_ = "error setting reading system attributes";
        lastError_ = SERVICE_ERROR;
        return lastError_;
    }

    // Session initialized
    LOG4CXX_INFO(onlineNodeLog, "Session to Daisy Online service established");
    loggedIn_ = true;
    good_ = true;
    lastError_ = OK;
    return lastError_;
}

DaisyOnlineNode::errorType DaisyOnlineNode::autoIssueContentList()
{
    LOG4CXX_INFO(onlineNodeLog, "get content list with new items");

    // get content list containing new items
    kdo::ContentList* content_list_new = pDOHandler->getContentList("new", 0, -1);

    // check if invoked operation caused errors
    if (!pDOHandler->good())
    {
        LOG4CXX_ERROR(onlineNodeLog, "Error occurred when invoking getContentList, " << pDOHandler->getStatus() << " '" << pDOHandler->getStatusMessage() << "'");
        return faultHandler(pDOHandler->getStatus());
    }
    if (content_list_new == NULL)
    {
        LOG4CXX_WARN(onlineNodeLog, "getContentList failed, returned contentList is NULL");
        errorstring_ = "error getting list with new content";
        lastError_ = SERVICE_ERROR;
        return lastError_;
    }

    // issue all new content items
    int numIssued = 0;
    errorType issueContentListResult = issueContentList(content_list_new, numIssued);
    if (issueContentListResult != OK)
    {
        LOG4CXX_ERROR(onlineNodeLog, "One or more content items could not be issued");
        return issueContentListResult;
    }
    if (numIssued > 0)
    {
        if (numIssued == 1)
        {
            Narrator::Instance()->setParameter("1", numIssued);
            Narrator::Instance()->play(_N("found {1} new publication"));
            Narrator::Instance()->playShortpause();
        }
        else
        {
            Narrator::Instance()->setParameter("2", numIssued);
            Narrator::Instance()->play(_N("found {2} new publications"));
            Narrator::Instance()->playShortpause();
        }
    }

    lastError_ = OK;
    return lastError_;
}

DaisyOnlineNode::errorType DaisyOnlineNode::issueContentList(kdo::ContentList* contentList, int& numIssued)
{
    numIssued = 0;
    errorstring_ = "";

    clock_t timeUntilNextIssue;
    time_t starttime, timenow;
    time(&starttime);

    std::vector<kdo::ContentItem> contentItems = contentList->getContentItems();
    const int numberOfContentItems = contentItems.size();
    LOG4CXX_INFO(onlineNodeLog, "Issuing all items in content list");
    LOG4CXX_DEBUG(onlineNodeLog, "Content list contains " << numberOfContentItems << " items");

    for (int i = numberOfContentItems - 1; i >= 0; i--)
    {
        LOG4CXX_INFO(onlineNodeLog, "processing item #" << i << " with id " << contentItems[i].getId());

        // when 3 seconds has passed and more than 1 item is left, play wait jingle
        time(&timenow);
        if ((i > 1) && (timenow - starttime > 3))
        {
            Narrator::Instance()->playWait();
        }

        // wait until we can issue this item unless it's the first item in contentList
        if (i < numberOfContentItems - 1)
        {
            while (clock() < timeUntilNextIssue)
            {
            }
        }

        // getContentMetadata for the content item, this operation is part of the issue process
        kdo::ContentMetadata *contentMetadata = pDOHandler->getContentMetadata(contentItems[i].getId());
        if (!pDOHandler->good())
        {
            LOG4CXX_ERROR(onlineNodeLog, "Error occurred when invoking getContentMetadata, " << pDOHandler->getStatus() << " '" << pDOHandler->getStatusMessage() << "'");
            return faultHandler(pDOHandler->getStatus());
        }
        if (contentMetadata == NULL)
        {
            LOG4CXX_WARN(onlineNodeLog, "getContentMetadata failed, return contentMetadata is NULL");
            errorstring_ = "failed to retrieve metadata for " + string(contentItems[i].getId());
            lastError_ = SERVICE_ERROR;
            return lastError_;
        }

        // issue the content item
        bool issueResult = pDOHandler->issueContent(contentItems[i].getId());
        if (!pDOHandler->good())
        {
            LOG4CXX_ERROR(onlineNodeLog, "Error occurred when invoking issueContent, " << pDOHandler->getStatus() << " '" << pDOHandler->getStatusMessage() << "'");
            return faultHandler(pDOHandler->getStatus());
        }
        if (!issueResult)
        {
            LOG4CXX_WARN(onlineNodeLog, "issueContent failed, service returned false");
            errorstring_ = "failed to issue item " + string(contentItems[i].getId());
            lastError_ = SERVICE_ERROR;
            return lastError_;
        }

        // set a clock time when next item can be issued
        timeUntilNextIssue = clock() + CLOCKS_PER_SEC;

        numIssued++;
    }

    if (numIssued == numberOfContentItems)
    {
        LOG4CXX_DEBUG(onlineNodeLog, "All items in content list were issued");
    }

    lastError_ = OK;
    return lastError_;
}

DaisyOnlineNode::errorType DaisyOnlineNode::autoCreateBookNodes()
{
    LOG4CXX_INFO(onlineNodeLog, "get content list with issued items");

    // get content list containing issued items
    kdo::ContentList* content_list_issued = pDOHandler->getContentList("issued", 0, -1);

    // check if invoked operation caused errors
    if (!pDOHandler->good())
    {
        LOG4CXX_ERROR(onlineNodeLog, "Error occurred when invoking getContentList, " << pDOHandler->getStatus() << " '" << pDOHandler->getStatusMessage() << "'");
        return faultHandler(pDOHandler->getStatus());
    }
    if (content_list_issued == NULL)
    {
        LOG4CXX_WARN(onlineNodeLog, "getContentList failed, returned contentList is NULL");
        errorstring_ = "error getting list with issued content";
        lastError_ = SERVICE_ERROR;
        return lastError_;
    }

    return createBookNodes(content_list_issued);
}

DaisyOnlineNode::errorType DaisyOnlineNode::createBookNodes(kdo::ContentList* contentList)
{
    int fail = 0;
    int added = 0;

    clearNodes();
    navilist.items.clear();

    errorstring_ = "";

    time_t starttime, timenow;
    time(&starttime);

    std::vector<kdo::ContentItem> contentItems = contentList->getContentItems();
    const int numberOfContentItems = contentItems.size();
    LOG4CXX_INFO(onlineNodeLog, "Creating book nodes for all items in content list");
    LOG4CXX_DEBUG(onlineNodeLog, "Content list contains " << numberOfContentItems << " items");

    for (int i = 0; i < numberOfContentItems; i++)
    {
        LOG4CXX_INFO(onlineNodeLog, "processing item #" << i << " with id " << contentItems[i].getId());

        // when 3 seconds has passed and more than 1 item is left, play wait jingle
        time(&timenow);
        if ((i + 1 < numberOfContentItems) && (timenow - starttime > 3))
        {
            Narrator::Instance()->playWait();
        }

        // join id and text string to avoid duplicates in database
        std::string name = contentItems[i].getId() + "_" + contentItems[i].getLabel().getText();

        // create book node if it has not already been added as a child
        stringstream uri_from_anything;
        uri_from_anything << "publication_" << reinterpret_cast<long>(contentItems[i].getLabel().getText().c_str());
        LOG4CXX_DEBUG(onlineNodeLog, "Invented uri for book: '" << uri_from_anything.str() << "'");

        // create node
        LOG4CXX_DEBUG(onlineNodeLog, "Creating book node: '" << name << "'");
        DaisyOnlineBookNode* node = new DaisyOnlineBookNode(contentItems[i].getId(), pDOHandler);
        node->name_ = name;
        node->uri_ = uri_from_anything.str(); // There is nothing that works like an uri so I used the address;

        // add node
        addNode(node);
        added++;

        // insert label audio in database
        insertLabelInMessageDb(contentItems[i]);

        // create a NaviListItem and store it in list for the NaviList signal
        NaviListItem item(uri_from_anything.str(), contentItems[i].getLabel().getText());
        navilist.items.push_back(item);
    }

    if (fail > 0)
    {
        LOG4CXX_WARN(onlineNodeLog, "Could not create " << fail << " book nodes of " << numberOfContentItems << " items in content list");
        lastError_ = SERVICE_ERROR;
        return lastError_;
    }

    LOG4CXX_DEBUG(onlineNodeLog, added << " book nodes added");

    serviceUpdated_ = true;
    lastError_ = OK;
    return lastError_;
}

DaisyOnlineNode::errorType DaisyOnlineNode::faultHandler(DaisyOnlineHandler::status status)
{
    switch (status)
    {
    case DaisyOnlineHandler::STATUS_SUCCESS:
        lastError_ = OK;
        return lastError_;
    case DaisyOnlineHandler::FAULT_INTERNALSERVERERROR:
        errorstring_ = pDOHandler->getLastSoapFaultReason();
        LOG4CXX_WARN(onlineNodeLog, "service returned InternalServerErrorFault with reason '" << errorstring_ << "'");
        lastError_ = SERVICE_ERROR;
        return lastError_;
    case DaisyOnlineHandler::FAULT_INVALIDOPERATION:
        errorstring_ = pDOHandler->getLastSoapFaultReason();
        LOG4CXX_WARN(onlineNodeLog, "service returned InvalidOperationFault with reason '" << errorstring_ << "'");
        lastError_ = SERVICE_ERROR;
        return lastError_;
    case DaisyOnlineHandler::FAULT_INVALIDPARAMETER:
        errorstring_ = pDOHandler->getLastSoapFaultReason();
        LOG4CXX_WARN(onlineNodeLog, "service returned InvalidParameterFault with reason '" << errorstring_ << "'");
        lastError_ = SERVICE_ERROR;
        return lastError_;
    case DaisyOnlineHandler::FAULT_NOACTIVESESSION:
        errorstring_ = pDOHandler->getLastSoapFaultReason();
        LOG4CXX_WARN(onlineNodeLog, "service returned NoActiveSessionFault with reason '" << errorstring_ << "'");
        lastError_ = SERVICE_ERROR;
        return lastError_;
    case DaisyOnlineHandler::FAULT_OPERATIONNOTSUPPORTED:
        errorstring_ = pDOHandler->getLastSoapFaultReason();
        LOG4CXX_WARN(onlineNodeLog, "service returned OperationNotSupportedFault with reason '" << errorstring_ << "'");
        lastError_ = SERVICE_ERROR;
        return lastError_;
    default:
        errorstring_ = pDOHandler->getStatusMessage();
        lastError_ = NETWORK_ERROR;
        return lastError_;
    }
}

bool DaisyOnlineNode::onOpen(NaviEngine& navi)
{
    if (loggedIn_ && serviceUpdated_ && navi.good())
    {
        currentChild_ = navi.getCurrentChoice();
        announce();
        return true;
    }

    // start chain of commands, this action will try to establish a session, issue
    // new content, and retrieve the content list
    sessionInit_signal();

    good_ = true;
    return true;
}

void DaisyOnlineNode::beforeOnOpen()
{
    if (not serviceUpdated_)
        Narrator::Instance()->play(_N("updating service"));
}

bool DaisyOnlineNode::process(NaviEngine& navi, int command, void* data)
{
    LOG4CXX_INFO(onlineNodeLog, "Processing command: " << command);

    switch (command)
    {
    case COMMAND_BACK:
        LOG4CXX_INFO(onlineNodeLog, "COMMAND_BACK received");
        return onOpen(navi);
    case COMMAND_RETRY_LOGIN_FORCED:
        // by setting lastLogOnAttempt_ to -1, session initialization will be forced
        lastLogOnAttempt_ = (DaisyOnlineNode::errorType)-1;
    case COMMAND_RETRY_LOGIN:
        sessionInit_signal();
        break;
    case COMMAND_DO_GETCONTENTLIST:
    {
        // get content list and create book nodes
        errorType autoResult = OK;
        autoResult = autoCreateBookNodes();
        if (autoResult != OK)
        {
            // announce result for auto create attempt
            announceResult(autoResult);

            // push COMMAND_HOME to command queue and return to the very beginning
            cq2::Command<ClientCore::COMMAND> c(ClientCore::HOME);
            c();
        }

        lastUpdate_ = time(NULL);
        navi.setCurrentChoice(firstChild());
        currentChild_ = firstChild();
        announce();

        if (numberOfChildren() == 1)
        {
            LOG4CXX_INFO(onlineNodeLog, "Opening the only child");
            // wait for narrator before sending command
            usleep(500000); while (Narrator::Instance()->isSpeaking()) usleep(100000);
            cq2::Command<INTERNAL_COMMAND> c(COMMAND_DOWN);
            c();
        }
    }
        break;

    default:
        LOG4CXX_INFO(onlineNodeLog, "Ignoring command: " << command);
        return false;
    };

    return true;
}

bool DaisyOnlineNode::insertLabelInMessageDb(kdo::ContentItem content)
{
    LOG4CXX_INFO(onlineNodeLog, "Download and insert audio label for content '" << content.getLabel().getText() << "' with id " << content.getId());

    if (not content.getLabel().hasAudio())
    {
        LOG4CXX_WARN(onlineNodeLog, "Title audio is missing for this content");
        return false;
    }

    // make an identifier for the content
    std::string identifier = content.getId() + "_" + content.getLabel().getText();

    /*
     * additional data we have access to
     *
     * content.getLabel().getAudio().getSize()
     * getLangCode( content.getLabel().getLang()
     */

    // check if audio for this content already have been added
    if (!Narrator::Instance()->hasOggAudio(identifier.c_str()))
    {
        // download and add audio to database
        char *audio_data = NULL;
        int audio_size = downloadData(content.getLabel().getAudio().getUri(), &audio_data);
        if (audio_size > 0)
        {
            bool result = Narrator::Instance()->addOggAudio(identifier.c_str(), audio_data, audio_size);
            free(audio_data);

            if (!result)
            {
                LOG4CXX_ERROR(onlineNodeLog, "Inserting audio data failed");
                return false;
            }
        }
        else
        {
            LOG4CXX_ERROR(onlineNodeLog, "Downloading audio data failed");
            return false;;
        }
    }

    return true;
}

size_t DaisyOnlineNode::downloadData(string uri, char **destinationbuffer)
{

    size_t bytes_read = 0;
    size_t bytes_read_total = 0;

    size_t bufsize = 8192 * 4;
    char* buffer = (char *) malloc(bufsize * sizeof(char));
    char* bufptr = buffer;

    LOG4CXX_DEBUG(onlineNodeLog, "Opening data stream for uri: " << uri);

    InputStream *is = NULL;
    is = DataStreamHandler::Instance()->newStream(uri, false, false);

    do
    {
        bytes_read = is->readBytes(bufptr, bufsize - bytes_read_total);
        if (bytes_read < 0)
        {
            LOG4CXX_ERROR(onlineNodeLog, "Failed to load data: " << is->getErrorMsg());
            free(buffer);
            return 0;
        }

        bytes_read_total += bytes_read;

        // Increase the buffer size if we need to
        if (bufsize - bytes_read_total <= 0)
        {
            buffer = (char *) realloc(buffer, bufsize * 2 * sizeof(char));
            if (buffer == NULL)
            {
                LOG4CXX_ERROR(onlineNodeLog, "Memory allocation error");
                return 0;
            }
            else
            {
                bufsize *= 2;
            }
        }
        bufptr = buffer + bytes_read_total;

    } while (bytes_read);

    *destinationbuffer = buffer;
    return bytes_read_total;
}

std::string DaisyOnlineNode::getLangCode(std::string language)
{
    std::string lang_str(language);
    if (lang_str.size() < 2)
    {
        LOG4CXX_WARN(onlineNodeLog, "language string '" << language << "' is too short");
        return "unknown";
    }

    std::string lang_code = Utils::toLower(lang_str.substr(0, 2));

    return lang_code;
}

void DaisyOnlineNode::announce()
{
    cq2::Command<NaviList> naviList(navilist);
    naviList();

    int numItems = numberOfChildren();

    if (numItems == 0)
    {
        Narrator::Instance()->play(_N("service contains no publications"));
    }
    else if (numItems == 1)
    {
        Narrator::Instance()->setParameter("1", numItems);
        Narrator::Instance()->play(_N("service contains {1} publication"));
    }
    else if (numItems > 1)
    {
        Narrator::Instance()->setParameter("2", numItems);
        Narrator::Instance()->play(_N("service contains {2} publications"));
    }
    Narrator::Instance()->playLongpause();

    announceSelection();
}

void DaisyOnlineNode::announceSelection()
{
    int currentChoice = 0;
    AnyNode* current = currentChild_;

    if ((firstChild() != NULL) && (current != NULL))
    {
        while (firstChild() != current)
        {
            currentChoice++;
            current = current->prev_;
        }

        Narrator::Instance()->setParameter("1", currentChoice + 1);
        Narrator::Instance()->play(_N("publication no. {1}"));
        Narrator::Instance()->play(currentChild_->name_.c_str());

        NaviListItem item = navilist.items[currentChoice];
        cq2::Command<NaviListItem> naviItem(item);
        naviItem();
    }
    else
    {
        // TODO: Investigate if this is actually a must to send this command with an empty NaviListItem
        NaviListItem item;
        cq2::Command<NaviListItem> naviItem(item);
        naviItem();
    }
}

void DaisyOnlineNode::announceResult(DaisyOnlineNode::errorType error)
{
    switch (error)
    {
    case NETWORK_ERROR:
    {
        // say error loading data
        Narrator::Instance()->play(_N("error loading data"));
        Narrator::Instance()->playLongpause();
        Narrator::Instance()->play(_N("retrying shortly"));
        ErrorMessage error(NETWORK, errorstring_);
        cq2::Command<ErrorMessage> message(error);
        message();
    }
        break;

    case SERVICE_ERROR:
    {
        // say error loading data
        Narrator::Instance()->play(_N("service error"));
        Narrator::Instance()->playLongpause();
        Narrator::Instance()->play(_N("retrying shortly"));
        ErrorMessage error(SERVICE, errorstring_);
        cq2::Command<ErrorMessage> message(error);
        message();
    }
        break;

    case USERNAME_PASSWORD_ERROR:
    {
        // say error logging in
        Narrator::Instance()->play(_N("username password error"));
        Narrator::Instance()->playLongpause();
        cq2::Command<NOTIFY_COMMAND> notify(NOTIFY_LOGIN_FAIL);
        notify();
    }
        break;

    default:
        break;
    }

    //If error occurred during login then send notification
    if(!loggedIn_){
        cq2::Command<NOTIFY_COMMAND> notify(NOTIFY_LOGIN_FAIL);
        notify();
    }
}
