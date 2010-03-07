#! /bin/bash

# Script for solving all example problems from directories EXAMPLES
# and BEISPIEL
#
# Usage: listDiffs.sh
#
# Run from an empty sibling directory of EXAMPLES and BEISPIEL.
#
# Uses: diff, ls

for f in `ls -tr *`; do echo $f; diff -wb $f ../REGRESSIONTESTS/$f; done
