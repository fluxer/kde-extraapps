#!/bin/sh
$XGETTEXT $(find . -name "*.cpp" -o -name "*.h") -o $podir/okular_md.pot
