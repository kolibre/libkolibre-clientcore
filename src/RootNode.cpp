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

#include "RootNode.h"
#include "DaisyOnlineNode.h"
#include "FileSystemNode.h"
#include "Defines.h"
#include "config.h"
#include "CommandQueue2/CommandQueue.h"
#include "Commands/InternalCommands.h"
#include "MediaSourceManager.h"
#include "Settings/Settings.h"
#include "Utils.h"

#include <Narrator.h>
#include <NaviEngine.h>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr rootNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.rootnode"));

using namespace naviengine;

RootNode::RootNode(const std::string useragent)
{
    LOG4CXX_TRACE(rootNodeLog, "Constructor");
    userAgent_ = useragent;
    name_ = "Root";
}

RootNode::~RootNode()
{
    LOG4CXX_TRACE(rootNodeLog, "Destructor");
}

// NaviEngine functions

bool RootNode::next(NaviEngine& navi)
{
    bool ret = MenuNode::next(navi);
    currentChild_ = navi.getCurrentChoice();
    announceSelection();
    return ret;
}

bool RootNode::prev(NaviEngine& navi)
{
    bool ret = MenuNode::prev(navi);
    currentChild_ = navi.getCurrentChoice();
    announceSelection();
    return ret;
}

bool RootNode::menu(NaviEngine& navi)
{
    return navi.openMenu(navi.buildContextMenu());
}

bool RootNode::up(NaviEngine& navi)
{
    bool ret = MenuNode::up(navi);
    return ret;
}

bool RootNode::onOpen(NaviEngine& navi)
{
    navi.setCurrentChoice(NULL);
    clearNodes();
    navilist_.items.clear();

    // Create sources defined in MediaSourceManager
    LOG4CXX_INFO(rootNodeLog, "Creating children for root");

    int daisyOnlineServices = MediaSourceManager::Instance()->getDaisyOnlineServices();
    if (daisyOnlineServices > 0)
    {
        // We will only support on DaisyOnlineService at the moment
        std::string name = MediaSourceManager::Instance()->getDOSname(0);
        std::string url = MediaSourceManager::Instance()->getDOSurl(0);
        std::string username = MediaSourceManager::Instance()->getDOSusername(0);
        std::string password = MediaSourceManager::Instance()->getDOSpassword(0);

        // Create a DaisyOnlineNode
        DaisyOnlineNode* daisyOnlineNode = new DaisyOnlineNode(name, url, username, password, userAgent_);
        if (daisyOnlineNode->good())
        {
            // Split userAgent into model/version
            std::string::size_type slashpos = userAgent_.find('/');
            if (slashpos != std::string::npos)
            {
                std::string model = userAgent_.substr(0, slashpos);
                std::string version = userAgent_.substr(slashpos + 1);
                if (not model.empty() && not version.empty())
                {
                    daisyOnlineNode->setModel(model);
                    daisyOnlineNode->setVersion(version);
                }
            }

            daisyOnlineNode->setLanguage(Settings::Instance()->read<std::string>("language", "sv"));
            LOG4CXX_INFO(rootNodeLog, "Adding DaisyOnlineService '" << name << "'");
            addNode(daisyOnlineNode);

            // create a NaviListItem and store it in list for the NaviList signal
            NaviListItem item(daisyOnlineNode->uri_, name);
            navilist_.items.push_back(item);
        }
        else
        {
            LOG4CXX_ERROR(rootNodeLog, "DaisyOnlineNode failed to initialize");
        }
    }

    int fileSystemPaths = MediaSourceManager::Instance()->getFileSystemPaths();
    for (int i = 0; i < fileSystemPaths; i++)
    {
        std::string name = MediaSourceManager::Instance()->getFSPname(i);
        std::string path = MediaSourceManager::Instance()->getFSPpath(i);

        if (Utils::isDir(path))
        {
            LOG4CXX_INFO(rootNodeLog, "Adding FileSystemNode '" << name << "'");
            FileSystemNode *fileSystemNode = new FileSystemNode(name, path);
            addNode(fileSystemNode);

            // create a NaviListItem and store it in list for the NaviList signal
            NaviListItem item(fileSystemNode->uri_, name);
            navilist_.items.push_back(item);
        }
    }

    if (navi.getCurrentChoice() == NULL && numberOfChildren() > 0)
    {
        navi.setCurrentChoice(firstChild());
        currentChild_ = firstChild();
    }

    currentChild_ = navi.getCurrentChoice();
    announce();

    if (numberOfChildren() == 1)
    {
        LOG4CXX_INFO(rootNodeLog, "Opening the only child");
        // wait for narrator before sending command
        usleep(500000); while (Narrator::Instance()->isSpeaking()) usleep(100000);
        cq2::Command<INTERNAL_COMMAND> c(COMMAND_DOWN);
        c();
    }

    return true;
}

bool RootNode::process(NaviEngine& navi, int command, void* data)
{
    return false;
}
bool RootNode::narrateInfo()
{
    const bool isSelfNarrated = true;
    Narrator::Instance()->play(_N("choose option using left and right arrows, open using play button"));
    Narrator::Instance()->playLongpause();
    announceSelection();
    return isSelfNarrated;
}

bool RootNode::onNarrate()
{
    const bool isSelfNarrated = true;
    return isSelfNarrated;
}

bool RootNode::onRender()
{
    const bool isSelfRendered = true;
    return isSelfRendered;
}

void RootNode::announce()
{
    cq2::Command<NaviList> naviList(navilist_);
    naviList();

    int numItems = numberOfChildren();

    if (numItems == 0)
    {
        Narrator::Instance()->play(_N("home contains no sources"));
    }
    else if (numItems == 1)
    {
        Narrator::Instance()->setParameter("1", numItems);
        Narrator::Instance()->play(_N("home contains {1} source"));
    }
    else if (numItems > 1)
    {
        Narrator::Instance()->setParameter("2", numItems);
        Narrator::Instance()->play(_N("home contains {2} sources"));
    }
    Narrator::Instance()->playLongpause();

    announceSelection();
}

void RootNode::announceSelection()
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
        Narrator::Instance()->play(_N("source no. {1}"));
        currentChild_->narrateName();

        NaviListItem item = navilist_.items[currentChoice];
        cq2::Command<NaviListItem> naviItem(item);
        naviItem();
    }
}
