#! /bin/sh

# Script for solving all example problems from directories EXAMPLES
# and BEISPIEL
#
# Usage: solveExamples.sh
#
# Run from an empty sibling directory of EXAMPLES and BEISPIEL.
#
# Uses: nice

if [ -d ../EXAMPLES ]; then
    for f in ../EXAMPLES/*inp
    do
        nice -19 ../py -maxmem 250M -regression $f
    done
else
    echo "Please create a sibling directory to EXAMPLES and run $0 from there"
fi

if [ -d ../BEISPIEL ]; then
    for f in ../BEISPIEL/*inp
    do
        nice -19 ../py -maxmem 250M -regression $f
    done
else
    echo "Please create a sibling directory to BEISPIEL and run $0 from there"
fi

echo -n "Solving examples ended at: "
date
