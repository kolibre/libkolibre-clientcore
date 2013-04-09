#!/bin/bash

## Copyright (C) 2012 Kolibre
#
# This file is part of kolibre-clientcore.
#
# Kolibre-clientcore is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# Kolibre-clientcore is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with kolibre-clientcore. If not, see <http://www.gnu.org/licenses/>.
#

PROMPTSFILE='prompts.csv'

usage()
{
    echo "usage: $0 [ACTION]"
    echo ""
    echo "[ACTION]"
    echo "  --missing   find and add missing prompts (default action)"
    echo "  --extra     find prompts that are not used"
}

find_prompt_in_string()
{
    local string=$1
    # TODO: allow optional spaces between marco and parantheses e.g. _N ( "one hour" )
    prompt=$(echo "$line" | sed 's/.*_N("\(.*\)".*).*/"\1"/')

    # now prompt variable should store a prompt inside double quotations e.g. "one hour"

    length_before=`expr length "$prompt"`
    prompt=$(echo "$prompt" | tr -d '\"')
    length_after=`expr length "$prompt"`

    # if ( length_before == (length_after + 2) ) then
    # the prompt variable store a valid prompt string
    # otherwise we set prompt to an empty string

    compare_value=`expr $length_after + 2`
    if ! [ $length_before -eq $compare_value ]; then
        prompt=""
    fi

}

prompt_not_in_prompts_file()
{
    local prompt=$1
    if sed 's/#.*//' $PROMPTSFILE | grep "\"$prompt\"" &> /dev/null; then
        return 1
    else
        return 0
    fi
}

find_missing_prompts()
{
    FOUNDMISSINGPROMPT=0
    TMPFILE="/tmp/$(basename $0).$$.tmp"

    # TODO: allow optional spaces between macro and parantheses e.g. _N ( "one hour" )
    find ../src/ -name '*.cpp' -exec grep -Hn "_N(.*)" {} \; > $TMPFILE

    while read line; do

        file=`echo $line | sed 's/\(.*cpp:[0-9]\+:\)\(.*\)/\1/' | sed 's/:[0-9]\+://'`
        code=`echo $line | sed 's/\(.*cpp:[0-9]\+:\)\(.*\)/\2/' | tr -d ' '`

        # continue if file include substring /tests/
        [[ "$file" =~ "/tests/" ]] && continue

        # continue if code begins with substring //
        [[ "$code" =~ "//" ]] && continue

        find_prompt_in_string $line

        # continue if string length of prompt is zero
        if [ -z "$prompt" ]; then
            continue
        fi

        if prompt_not_in_prompts_file "$prompt"; then
            echo "prompt '$prompt' is missing in $PROMPTSFILE"
            if [ $FOUNDMISSINGPROMPT -eq 0 ]; then
                datestr=`date`
                echo "" >> $PROMPTSFILE
                echo "# prompts added on $datestr" >> $PROMPTSFILE
                echo "" >> $PROMPTSFILE
                FOUNDMISSINGPROMPT=1
            fi
            echo "\"$prompt\"" >> $PROMPTSFILE
        fi
    done < $TMPFILE

    rm $TMPFILE

    exit $FOUNDMISSINGPROMPT
}

find_extra_prompts()
{
    FOUNDEXTRAPROMPT=0

    while read line; do

        # continue if line begins with #
        [[ "$line" =~ "#" ]] && continue

        # strip leading and trailing white spaces
        prompt=`echo $line | tr -d ' '`
        prompt=$(echo $line | sed -e 's/^ *//g;s/ *$//g')

        # continue if string length of prompt is zero
        if [ -z "$prompt" ]; then
            continue
        fi

        if ! grep -r "$prompt" ../src -q; then
            if [ $FOUNDEXTRAPROMPT -eq 0 ]; then
                echo "List of prompts which are not used"
                FOUNDEXTRAPROMPT=1
            fi
            echo "$prompt"
        fi
    done < $PROMPTSFILE
}

if [ ! -f $PROMPTSFILE ]; then
    echo "error: could not find file '$PROMPTSFILE'"
    exit 1
fi

if [ $# -eq 0 ] || [ "$1" == "--missing" ]; then
    find_missing_prompts
elif [ "$1" == "--extra" ]; then
    find_extra_prompts
elif [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    usage
    exit 0
else
    usage
    exit 1
fi
