#include "Utils.h"

#include <string>
#include <assert.h>
#include <iostream>

int main(int argc, char *argv[])
{
    std::string input;

    // leading and trailing whitespace
    input = " foo ";
    Utils::trim(input);
    assert("foo" == input);
    input = "  foo  ";
    Utils::trim(input);
    assert("foo" == input);

    // leading and trailing \n
    input = "\nfoo\n";
    Utils::trim(input);
    assert("foo" == input);
    input = "\n\nfoo\n\n";
    Utils::trim(input);
    assert("foo" == input);

    // leading and trailing \t
    input = "\tfoo\t";
    Utils::trim(input);
    assert("foo" == input);

    // leading and trailing \v
    input = "\vfoo\v";
    Utils::trim(input);
    assert("foo" == input);

    // leading and trailing \r
    input = "\rfoo\r";
    Utils::trim(input);
    assert("foo" == input);

    // leading and trailing \f
    input = "\ffoo\f";
    Utils::trim(input);
    assert("foo" == input);

    return 0;
}
