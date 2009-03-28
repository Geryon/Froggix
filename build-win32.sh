#!/bin/sh
# $Id: build-win32.sh,v 1.1 2009-03-28 03:52:29 nick Exp $

PROGNAME=froggix

if !(./cross-configure.sh) then exit 1; fi
./cross-make.sh clean
./cross-make.sh
if test $? -ne 0; then exit 1; fi
test -w src/$PROGNAME && chmod -x src/$PROGNAME
test -w src/$PROGNAME && mv src/$PROGNAME src/$PROGNAME.exe
