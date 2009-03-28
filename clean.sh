#!/bin/sh
# $Id: clean.sh,v 1.1 2009-03-28 03:52:29 nick Exp $

PROGNAME=froggix
FILES='	aclocal.m4
	config.cache
	config.cross.cache
	config.log
	config.status
	configure
	Makefile
	Makefile.in
	src/*.o
	src/Makefile
	src/Makefile.in
	src/$PROGNAME
	src/$PROGNAME.exe
	src/std*.txt'

test -w configure && (if (./configure) then make distclean; fi)
for file in $FILES; do test -w $file && rm -f $file; done
test -w src/.deps && rm -rf src/.deps
