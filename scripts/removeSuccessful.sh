#! /bin/bash

# Compare the generated solutions to thos stored in ../REGRESSIONTESTS
# Remove the matching files. In other words, the remaining files are
# those containing potential regressions.
# Differences in white space are ignored.
#
# Usage: removeSuccessful.sh
#
# Start from the directory containing the output files of a regression
# test (which should be as sibling directory of REGRESSIONTESTS).
#
# Uses: diff, rm

for f in *
do
    diff -wb $f ../REGRESSIONTESTS/$f > /dev/null 2> /dev/null && rm -f $f
done
