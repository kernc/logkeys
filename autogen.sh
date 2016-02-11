#! /bin/sh
set -e

# Use this script to bootstrap your build AFTER checking it out from
# source control. You should not have to use it for anything else.

# Runs autoconf, autoheader, aclocal, automake, autopoint, libtoolize
echo
echo "Regenerating autotools files ..."
aclocal \
&& autoheader \
&& automake --add-missing \
&& autoconf

echo "... done.  Now please do the following:"
echo
echo "   cd build; ../configure; make; su; make install"
echo
