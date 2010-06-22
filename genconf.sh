#!/bin/sh

bin_PATH="/usr/bin"

FIRST_BUILD=no

if [ ! -d build ] ; then
	mkdir -p build
	FIRST_BUILD=yes
fi

if [ -x $bin_PATH/aclocal ] && [ -x $bin_PATH/autoheader ] \
	&& [ -x $bin_PATH/automake ] && [ -x $bin_PATH/autoconf ]; then
	AUTOMAKE_OPTIONS=""
	if test "x${FIRST_BUILD}" == "xyes"; then
		AUTOMAKE_OPTIONS=" --add-missing --copy"
	fi
	aclocal && autoheader && automake $AUTOMAKE_OPTIONS && autoconf
fi

exit 0

