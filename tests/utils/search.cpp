#include "Utils.h"
#include "../setup_logging.h"

#include <string>
#include <vector>
#include <assert.h>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;

    // setup logging
    setup_logging();

    std::vector<std::string> matches;

    // search ncc.html files
    matches = Utils::recursiveSearchByFilename(argv[1], "ncc.html");
    assert(matches.size() == 2);

    // search *.html files
    matches = Utils::recursiveSearchByExtension(argv[1], ".html");
    std::cout << "matches.size() = " << matches.size() << std::endl;
    assert(matches.size() == 4);

    return 0;
}
