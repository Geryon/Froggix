#!/bin/sh
# $Id: build-linux.sh,v 1.1 2009-03-28 03:52:29 nick Exp $

if !(./configure) then exit 1; fi
make clean
make
