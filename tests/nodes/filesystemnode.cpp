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

#include "FileSystemNode.h"
#include "../setup_logging.h"

#include <NaviEngine.h>

#include <stdlib.h>
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

    Navi navi;

    // create a file system node with a no-existing path
    FileSystemNode *fileSystemNode = new FileSystemNode("name", "path");

    // we expect that file system node has no children
    assert(navi.openMenu(fileSystemNode));
    assert(fileSystemNode->onOpen(navi));
    assert(fileSystemNode->numberOfChildren() == 0);
    navi.closeMenu();

    // create a new file system node with an existing path
    char* srcdir = getenv("srcdir");
    std::string path = std::string(srcdir) + "/../utils/searchData";
    fileSystemNode = new FileSystemNode("name", path);

    // we expect that file system node has two children
    assert(navi.openMenu(fileSystemNode));
    assert(fileSystemNode->onOpen(navi));
    assert(fileSystemNode->numberOfChildren() == 2);

    // loop through each child an verify that all names and uris are unique
    std::set<std::string> names;
    std::set<std::string> uris;
    for (int i=0; i<fileSystemNode->numberOfChildren(); i++)
    {
        names.insert(navi.getCurrentChoice()->name_);
        uris.insert(navi.getCurrentChoice()->uri_);
        assert(fileSystemNode->next(navi));
    }
    assert(names.size() == fileSystemNode->numberOfChildren());
    assert(uris.size() == fileSystemNode->numberOfChildren());

    navi.closeMenu();

    return 0;
}
