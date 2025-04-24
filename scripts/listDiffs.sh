#! /bin/bash

# Script for listing the regressions found in a regression test
# 
# Start from the directory containing the output files of a regression test.
#
# Usage: listDiffs.sh
#
# Uses: diff, ls

SCRIPTDIR=$(dirname $0)
POPEYEDIR=${SCRIPTDIR}/..

for f in `ls -tr *.*`; do echo $f; diff -wb ${POPEYEDIR}/REGRESSIONTESTS/$f $f; done

