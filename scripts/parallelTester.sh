#!/bin/bash

# Run a regression test, using a configurable number of processors
#
# Run from the directory that is to contain the Popeye output.
#
# Pass -all as only option to also test the input files that take
# a loooooooooooong time.

SCRIPTDIR=$(dirname $0)
POPEYEDIR=${SCRIPTDIR}/..

. ${SCRIPTDIR}/parallelTester.lib

# command to be invoked in parallel
_cmd="${POPEYEDIR}/py -maxmem 1G -maxtrace 0 -regression"
#_cmd="WINEPREFIX=~/.wine wine ${POPEYEDIR}/pywin32.exe -maxmem 1G -maxtrace 0 -regression"
#_cmd="WINEPREFIX=~/.wine64 wine64 ${POPEYEDIR}/pywin64.exe -maxmem 1G -maxtrace 0 -regression"

# number of processors
PMAX=3

#DEBUG=1

# create and dispatch jobs
for item in ${POPEYEDIR}/TESTS/*.inp ${POPEYEDIR}/REGRESSIONS/*.inp ${POPEYEDIR}/EXAMPLES/*inp ${POPEYEDIR}/BEISPIEL/*inp; do
  echo "$item"
done | dispatchWork

if [ "$1" = "-all" ]
then
  for item in ${POPEYEDIR}/EXAMPLES/lengthy/*inp ${POPEYEDIR}/BEISPIEL/lengthy/*inp; do
    echo "$item"
  done | dispatchWork
fi

