#include "Utils.h"
#include "../setup_logging.h"

#include <string>
#include <assert.h>
#include <iostream>

int main(int argc, char *argv[])
{
    // setup logging
    setup_logging();

    assert(Utils::isFile(argv[0]));
    assert(Utils::isFile("/tmp") == false);
    assert(Utils::isFile(".") == false);

    return 0;
}
