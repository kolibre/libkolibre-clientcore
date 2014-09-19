#include "Utils.h"
#include "../setup_logging.h"

#include <string>
#include <assert.h>
#include <iostream>

int main(int argc, char *argv[])
{
    // setup logging
    setup_logging();

    assert(Utils::fileExtension("") == "");
    assert(Utils::fileExtension("foo") == "foo");
    assert(Utils::fileExtension("foo.bar") == "bar");
    assert(Utils::fileExtension("foo.foobar") == "bar");
    assert(Utils::fileExtension("FOO.BAR") == "bar");
    assert(Utils::fileExtension("Foo.bAr") == "bar");

    return 0;
}
