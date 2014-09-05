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
#include "MediaSourceManager.h"
#include "../setup_logging.h"

#include <NaviEngine.h>

#include <assert.h>
#include <set>

using namespace std;

class Navi: public naviengine::NaviEngine
{
    naviengine::MenuNode* buildContextMenu()
    {
        return NULL;
    }

    void narrateChange(const naviengine::NaviEngine::MenuState& before, const naviengine::NaviEngine::MenuState& after)
    {
    }

    void narrate(std::string message)
    {
    }

    void narrate(int value)
    {
    }

    void narrateStop()
    {
    }

    void narrateShortPause()
    {
    }

    void narrateLongPause()
    {
    }
};

int main(int argc, char **argv)
{
    // setup logging
    setup_logging();

    // clear all media sources
    MediaSourceManager::Instance()->clearDaisyOnlineServies();
    MediaSourceManager::Instance()->clearFileSystemsPaths();

    RootNode *rootNode = new RootNode();
    Navi navi;
    assert(navi.openMenu(rootNode));

    // we expect that rootNode has no children since no sources has been added
    assert(rootNode->onOpen(navi));
    assert(rootNode->numberOfChildren() == 0);

    // add two daisy online services
    MediaSourceManager::Instance()->addDaisyOnlineService("service1","url1","username","password");
    MediaSourceManager::Instance()->addDaisyOnlineService("service2","url2","username","password");

    // we expect that rootNode only has one child since we only support one DaisyOnlineSource at the moment
    assert(rootNode->onOpen(navi));
    assert(rootNode->numberOfChildren() == 1);

    // add two file system paths which does not exist
    MediaSourceManager::Instance()->addFileSystemPath("path1","/tmp/path1");
    MediaSourceManager::Instance()->addFileSystemPath("path2","/tmp/path2");

    // we expect that rootNode only has one child since none of the paths exists
    assert(rootNode->onOpen(navi));
    assert(rootNode->numberOfChildren() == 1);

    // add two file system paths which do exist
    MediaSourceManager::Instance()->addFileSystemPath("path3","/tmp");
    MediaSourceManager::Instance()->addFileSystemPath("path4","/tmp");

    // we expect that rootNode has three children
    assert(rootNode->onOpen(navi));
    assert(rootNode->numberOfChildren() == 3);

    // loop through each child an verify that all names and uris are unique
    std::set<std::string> names;
    std::set<std::string> uris;
    for (int i=0; i<rootNode->numberOfChildren(); i++)
    {
        names.insert(navi.getCurrentChoice()->name_);
        uris.insert(navi.getCurrentChoice()->uri_);
        assert(rootNode->next(navi));
    }
    assert(names.size() == rootNode->numberOfChildren());
    assert(uris.size() == rootNode->numberOfChildren());

    return 0;
}
