#!/bin/bash

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb --batch --return-child-result -x ${srcdir:-.}/run --args"
fi

${PREFIX} ${bindir:-.}/daisybooknode ${srcdir:-.}/testdata/FireSafety/ncc.html
