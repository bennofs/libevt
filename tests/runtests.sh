#!/bin/sh
# Script to run tests
#
# Version: 20201121

if test -f ${PWD}/libevt/.libs/libevt.1.dylib && test -f ./pyevt/.libs/pyevt.so;
then
	install_name_tool -change /usr/local/lib/libevt.1.dylib ${PWD}/libevt/.libs/libevt.1.dylib ./pyevt/.libs/pyevt.so;
fi

make check CHECK_WITH_STDERR=1;
RESULT=$?;

if test ${RESULT} -ne 0 && test -f tests/test-suite.log;
then
	cat tests/test-suite.log;
fi
exit ${RESULT};

