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

#ifndef _DAISYONLINENODE_H
#define _DAISYONLINENODE_H

#include "NaviList.h"

#include <DaisyOnlineHandler.h>
#include <Nodes/MenuNode.h>

#include <string>
#include <boost/signals2.hpp>

/**
 * DaisyOnlineNode implements the MenuNode, making a publication list
 * (library) function like a menu.
 */
class DaisyOnlineNode: public naviengine::MenuNode
{
public:
    DaisyOnlineNode(const std::string uri, const std::string username, const std::string password, const std::string& client_home, std::string useragent = "");
    ~DaisyOnlineNode();

    bool good();
    std::string getErrorMessage();

    // ReadingSystemAttributes values
    void setManufacturer(const std::string &manufacturer)
    {
        manufacturer_ = manufacturer;
    }
    ;
    void setModel(const std::string &model)
    {
        model_ = model;
    }
    ;
    void setSerialNumber(const std::string &serialNumber)
    {
        serialNumber_ = serialNumber;
    }
    ;
    void setVersion(const std::string &version)
    {
        version_ = version;
    }
    ;
    void setLanguage(const std::string &language)
    {
        language_ = language;
    }
    ;

    // MenuNode start
    // virtual methods from naviengine::AnyNode
    bool menu(naviengine::NaviEngine&);
    bool onOpen(naviengine::NaviEngine&);
    bool process(naviengine::NaviEngine&, int command, void* data = 0);
    bool next(naviengine::NaviEngine&);
    bool prev(naviengine::NaviEngine&);
    bool up(naviengine::NaviEngine&);
    bool onNarrate();
    bool onRender();

    enum errorType
    {
        OK,
        NETWORK_ERROR,
        SERVICE_ERROR,
        USERNAME_PASSWORD_ERROR,
    };
    errorType getLastError();

private:
    bool good_; // if false, call getErrorMessage, resets on every invoke
    DaisyOnlineHandler *pDOHandler;
    const std::string serverUrl_;
    std::string username_;
    std::string password_;
    std::string previousUsername_;
    std::string previousPassword_;
    std::string useragent_;
    std::string manufacturer_;
    std::string model_;
    std::string serialNumber_;
    std::string version_;
    std::string language_;

    bool firstChildNotOpened_;
    bool loggedIn_;
    NaviList navilist;
    AnyNode* currentChild_;

    time_t lastUpdate_;
    errorType lastError_;
    errorType lastLogOnAttempt_;
    std::string errorstring_;

    // signals and slots to control invocation of DaisyOnline calls
    boost::signals2::signal<void()> sessionInit_signal;
    boost::signals2::signal<void()> issueContent_signal;
    void onSessionInit();
    void onIssueContent();
    // signals and slots to control invocation of DaisyOnline calls end

    DaisyOnlineNode::errorType sessionInit();
    DaisyOnlineNode::errorType autoIssueContentList();
    DaisyOnlineNode::errorType issueContentList(kdo::ContentList* contentList, int& numIssued);
    DaisyOnlineNode::errorType autoCreateBookNodes();
    DaisyOnlineNode::errorType createBookNodes(kdo::ContentList* contentList);
    DaisyOnlineNode::errorType faultHandler(DaisyOnlineHandler::status status);

    // functions for inserting label audio into messages.db
    bool insertLabelInMessageDb(kdo::ContentItem);
    size_t downloadData(std::string uri, char **destinationbuffer);

    std::string getLangCode(std::string language);
    void announce();
    void announceSelection();
    void announceResult(DaisyOnlineNode::errorType error);
};

#endif
