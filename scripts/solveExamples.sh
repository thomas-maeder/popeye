#! /bin/bash

# Script for solving all example problems from directories REGRESSIONS,
# EXAMPLES and BEISPIEL
#
# Usage: solveExamples.sh
#
# Uses: nice

SCRIPTDIR=$(dirname $0)
POPEYEDIR=${SCRIPTDIR}/..

ulimit -f 200

if [ -d ${POPEYEDIR}/REGRESSIONS ]; then
    for f in ../REGRESSIONS/*inp
    do
        ${POPEYEDIR}/py -maxmem 1G -regression -maxtrace 0 $f
    done
else
    echo "Please create a sibling directory to REGRESSIONS and run $0 from there"
fi

if [ -d ${POPEYEDIR}/EXAMPLES ]; then
    for f in ${POPEYEDIR}//EXAMPLES/*inp
    do
        ${POPEYEDIR}/py -maxmem 1G -regression -maxtrace 0 $f
    done
else
    echo "Please create a sibling directory to EXAMPLES and run $0 from there"
fi

if [ -d ${POPEYEDIR}/BEISPIEL ]; then
    for f in ${POPEYEDIR}//BEISPIEL/*inp
    do
        ${POPEYEDIR}/py -maxmem 1G -regression -maxtrace 0 $f
    done
else
    echo "Please create a sibling directory to BEISPIEL and run $0 from there"
fi

echo -n "Solving examples ended at: "
date
