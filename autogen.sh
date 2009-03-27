#!/bin/sh
#$Id: autogen.sh,v 1.2 2009-03-27 19:50:44 nick Exp $

aclocal
automake --foreign --add-missing
autoconf
