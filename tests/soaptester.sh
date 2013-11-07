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

if [ $# -lt 2 ]; then
    echo "usage: $0 <test case> <test data> [test parameters]"
    echo "       test case indicates which test to run e.g. nodes/daisyonlinebooknode"
    echo "       test data indicates which folder to use for test data e.g. nodes/daisyOnlineBookNode"
    echo "       test parameters are not currently supported"
    echo
    echo "note:  specify full path for parameters <test case> and <test data>"
    echo
    exit 1
fi

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb --batch --return-child-result -x ${srcdir:-.}/run --args"
fi

# run with valgrind
if [ $# -ge 3 ] && [ $3 = 'valgrind' ]; then
    if [ -x /usr/bin/valgrind ]; then
        PREFIX="libtool --mode=execute valgrind --leak-check=full --tool=memcheck --suppressions=${srcdir:-.}/memoryleak.supp"
    fi
fi

if [ ! -f $1 ]; then
    echo "test case '$1' does not exist"
    exit 1
fi

if [ ! -d $2 ]; then
    echo "test data '$2' does not exist"
    exit 1
fi

start_fakesoapserver()
{
    local soapserver=$1
    local protocol=$2
    local port=$3
    local input=$4
    local ordefile=$5

    if [ "$soapserver" == 'fakesoapserver.py' ]; then
        start_python $protocol $port $input $orderfile
    else
        start_socat $protocol $port $input $orderfile
    fi
    sleep 1
}

start_python()
{
    local protocol=$1
    local port=$2
    local input=$3
    local ordefile=$4

    file=`readlink -f $0`
    path=`dirname $file`
    fakesoap="${path}/fakesoapserver.py"
    if [ $protocol = 'http' ]; then
        $fakesoap -p $port -i $input -o $orderfile &
    else
        echo "you must create a combined certificate and privete key file in order to use https"
        exit 2
        cert="${path}/ssl-server-pem.pem"
        $fakesoap -p $port -i $input -o $orderfile -s -c $cert &
    fi
}

start_socat()
{
    echo "this fake server is not supported, please use fakesoapserver.py instead"
    exit 2

    socat=$(which socat)
    if [ $? -ne 0 ]; then
        echo "socat not found, please install package socat"
        exit 1
    fi

    local protocol=$1
    local port=$2
    local input=$3
    local orderfile=$4

    file=`readlink -f $0`
    path=`dirname $file`
    fakesoap="${path}/fakesoapresponder.sh"
    if [ $protocol = 'http' ]; then
        socat TCP4-LISTEN:$PORT,fork,tcpwrap=script EXEC:"$fakesoap $input $orderfile" &
    else
        echo "you must create a certificate file and a private key file in order to use https"
        exit 2
        cert="${path}/ssl-server-crt.crt"
        key="${path}/ssl-private-key.key"
        socat OPENSSL-LISTEN:$PORT,fork,tcpwrap=script,cert=$cert,key=$key,verify=0 EXEC:"$fakesoap $input $orderfile" &
    fi
}


kill_fakesoapserver()
{
    local soapserver=$1
    local port=$2

    line=`ps ax | grep $soapserver | grep $port`
    pid=`echo $line | cut -d ' ' -f1`
    kill $pid
}

unset_axis2c_home()
{
    if ! [ -z "$AXIS2C_HOME" ]; then
        echo "Setting AXIS2C_HOME forces logs to be written to that dir, so don't!"
        exit 1
    fi
}

port_in_use()
{
    local port=$1

    #echo "checking if port $port is in use"
    output=`lsof -i -P -n | grep ":$port"`
    if [ ! -z "$output" ]; then
        return 0 # port is in use
    fi
    return 1 # port is not in use
}

unset_axis2c_home

# find available port
PORT=$$
if [ $PORT -lt 1025 ]; then
    PORT=`expr $PORT + 1024`
fi

while port_in_use $PORT; do
    echo "looping"
    PORT=`expr $PORT + 1`
done

# use http as default protocol (other options: 'https')
PROTOCOL='http'

# use python fakesoapserver as default fake server (other options: 'socat')
SOAPSERVER='fakesoapserver.py'

orderfile="${2##*/}.order"
echo -n 1 > $orderfile

# setup test parameters
uri="$PROTOCOL://localhost:$PORT"
username="test"
password="test"
binary=$1
input=$2

start_fakesoapserver $SOAPSERVER $PROTOCOL $PORT $input $orderfile

$PREFIX $binary $uri $username $password $orderfile
retval=$?

kill_fakesoapserver $SOAPSERVER $PORT

if [ $retval = 0 ]
then
    exit 0
else
    exit -1
fi
