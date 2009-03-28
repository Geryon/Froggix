#!/bin/sh
# $Id: autogen.sh,v 1.4 2009-03-28 03:59:07 nick Exp $

# Remove any previously created cache files
test -w config.cache && rm config.cache
test -w config.cross.cache && rm config.cross.cache

# Regenerate configuration files
aclocal
automake --foreign --add-missing
autoconf

# Run configure for this platform
#./configure $*
echo "Now you are ready to run ./configure"
