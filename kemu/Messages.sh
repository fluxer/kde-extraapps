#! /bin/sh
$EXTRACTRC $(find . -name '*.rc') >> rc.cpp || exit 11
$EXTRACTRC $(find . -name '*.ui') >> rc.cpp || exit 12
$XGETTEXT kded/*.cpp krunner/*.cpp *.cpp rc.cpp -o $podir/kemu.pot
rm -f rc.cpp
