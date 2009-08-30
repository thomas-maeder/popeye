#!/bin/bash

. `dirname $0`/parallelTester.lib

# command to be invoked in parallel
_cmd="../py -maxmem 1G -maxtrace 0 -regression"

# number of processors
PMAX=3

# create and dispatch jobs
for item in ../TESTS/*.inp ../REGRESSIONS/*.inp ../EXAMPLES/*inp ../BEISPIEL/*inp; do
  echo "$item"
done | dispatchWork
