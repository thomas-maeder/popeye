#! /bin/bash

# Helper script for repeatedly solving a test file and printing out
# the time spent in user space
#
# Usage: timing.sh <testfile> <nrofruns> [<memory> [<filter>]]
#
# <memory> defaults to 250M
#
# <filter> defaults to user which is nice on Linux; on Cygwin, you
# probably want to use real instead
#
# Uses: time, grep

testfile=$1
nrofruns=$2
memory=$3
filter=$4

if [ -z $memory ]
then
    memory=250M
fi

if [ -z $filter ]
then
    filter=user
fi

for ((i=1; $i<=$nrofruns; i=$((i+1))))
do
    time ./py -maxmem $memory $testfile
done 2>&1 | grep -w $filter
