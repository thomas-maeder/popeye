#! /bin/bash

# Check again the example files that produced the output files
# currently in the current directy.
#
# Usage: checkAgain.sh
#
# Start from the directory containing the output files of a regression
# test (which should be a sibling directory of BEISPIEL and EXAMPLES).
#
# Uses: sed

for f in *.ref
do
    stem=`echo $f | sed -e 's/[.]ref$//'`
    inputfile=../EXAMPLES/$stem.inp
    ../py -maxmem 250M -regression -notrace $inputfile
done

for f in *.out
do
    stem=`echo $f | sed -e 's/[.]out$//'`
    inputfile=../BEISPIEL/$stem.inp
    ../py -maxmem 250M -regression -notrace $inputfile
done
