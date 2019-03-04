#!/bin/sh
libtoolize
autoreconf
aclocal
autoheader
automake -a -c --foreign
autoconf

