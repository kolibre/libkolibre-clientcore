#include "Utils.h"

#include <string>
#include <assert.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;

    std::string expected = argv[1];
    assert(Utils::getDatapath() == expected);
    return 0;
}
