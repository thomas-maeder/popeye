#!/bin/bash

# Run a regression test, using a configurable number of processors
#
# Run from the directory that is to contain the Popeye output.

SCRIPTDIR=$(dirname $0)
POPEYEDIR=${SCRIPTDIR}/..

. ${SCRIPTDIR}/parallelTester.lib

# command to be invoked in parallel
_cmd="${POPEYEDIR}/py -maxmem 1G -maxtrace 0 -regression"

# number of processors
PMAX=3

# create and dispatch jobs
for item in ${POPEYEDIR}/TESTS/*.inp ${POPEYEDIR}/REGRESSIONS/*.inp ${POPEYEDIR}/EXAMPLES/*inp ${POPEYEDIR}/BEISPIEL/*inp; do
  echo "$item"
done | dispatchWork
