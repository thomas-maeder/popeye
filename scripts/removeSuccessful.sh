#! /bin/bash

# Compare the generated solutions to thos stored in ../REGRESSIONTESTS
# Remove the matching files. In other words, the remaining files are
# those containing potential regressions.
# Differences in white space are ignored.
#
# Usage: removeSuccessful.sh
#
# Start from the directory containing the output files of a regression test.
#
# Uses: diff, rm

SCRIPTDIR=$(dirname $0)
POPEYEDIR=${SCRIPTDIR}/..

for f in *
do
    diff -q -wb $f ${POPEYEDIR}/REGRESSIONTESTS/$f > /dev/null && rm -f $f
done
