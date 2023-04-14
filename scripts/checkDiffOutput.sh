#! /bin/bash

# helper script for parallel execution of checkOutput.sh
# compare output of checkOutput.sh to original file - print file name if there is a difference

(scripts/checkOutput.tcl $1 | diff $1 - > /dev/null) || echo $1
