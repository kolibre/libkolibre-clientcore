#!/bin/bash

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb -return-child-result -x ${srcdir:-.}/run --args"
fi

${PREFIX} ${bindir:-.}/daisynavi ${srcdir:-.}/daisyNavi/FireSafety/ncc.html
