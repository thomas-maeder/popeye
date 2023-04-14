#!/bin/bash

# Compare output files to the result of applying checkOuptut.sh to them
# Print names of output files where there is a difference.
#
# Run from the directory that is to contain the Popeye output.
#
# Pass -all as only option to test all output files

SCRIPTDIR=$(dirname $0)
POPEYEDIR=${SCRIPTDIR}/..

. ${SCRIPTDIR}/parallelTester.lib

# command to be invoked in parallel
_cmd="${POPEYEDIR}/scripts/checkDiffOutput.sh"

NRCPUS=$(cat /proc/cpuinfo | grep processor | tail --lines=1 | tr '\t' ' ' | tr --squeeze-repeats ' ' | cut --delimiter=' ' --fields=3)

PMAX=$(($NRCPUS-2))
if [[ $PMAX<0 ]]
then
    PMAX=1
fi

#DEBUG=1

# create and dispatch jobs
for item in $*
do
  echo "$item"
done | dispatchWork

if [ "$1" = "-all" ]
then
    for item in ${POPEYEDIR}/REGRESSIONTESTS//*
    do
	echo "$item"
    done | dispatchWork
fi

