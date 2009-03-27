#!/bin/sh
#$Id: autogen.sh,v 1.1 2009-03-27 19:47:27 nick Exp $

aclocal
automake --foreign
autoconf
