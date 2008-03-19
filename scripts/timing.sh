#! /bin/bash

# Helper script for repeatedly solving a test file and printing out
# the time spent in user space
#
# Usage: timing.sh <testfile> <nrofruns> [<filter>]
#
# <filter> defaults to user which is nice on Linux; on Cygwin, you
# probably want to use real instead
#
# Uses: time, grep

testfile=$1
nrofruns=$2
filter=$3

if [ -z $filter ]
then
    filter=user
fi

for ((i=1; $i<=$nrofruns; i=$((i+1))))
do
    time ./py -maxmem 250M $testfile
done 2>&1 | grep -w $filter
