#! /bin/sh

# Helper script for repeatedly solving a test file and printing out
# the time spent in user space
#
# Usage: timing.sh <testfile> <nrofruns>
#
# Uses: time, grep

testfile=$1
nrofruns=$2

for ((i=1; $i<=$nrofruns; i=$((i+1))))
do
    time ./py -maxmem 250M $testfile
done 2>&1 | grep -w user
