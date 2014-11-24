#!/bin/sh

set -e
boost=""
for dir in "/include" "/usr/include" "/usr/local";do
    if [ -d "$dir/boost" ];then
        boost="$dir"
    fi
done
if [ -z "$boost" ];then
    echo "ERROR: unable to find boost headers"
    exit 1
elif [ -z "$(type -p bcp)" ];then
    "ERROR: bcp tool not found"
    exit 2
fi
rm -vrf miniboost/
mkdir -vp miniboost/

bcp --boost="$boost" boost/shared_array.hpp boost/concept_check.hpp \
    boost/scoped_ptr.hpp boost/circular_buffer.hpp \
    miniboost/