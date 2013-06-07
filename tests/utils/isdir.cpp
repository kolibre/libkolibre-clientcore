#include "Utils.h"
#include "../setup_logging.h"

#include <string>
#include <assert.h>
#include <iostream>

int main(int argc, char *argv[])
{
    // setup logging
    setup_logging();

    assert(Utils::isDir("/tmp"));
    assert(Utils::isDir("."));
    assert(Utils::isDir(argv[0]) == false);
    assert(Utils::isDir("/foo/bar") == false);

    return 0;
}
