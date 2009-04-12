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

for f in *.reg
do
    # only true if *.ref isn't expanded because there is no matching file
    if [ -f $f ]
    then
        stem=`echo $f | sed -e 's/[.]ref$//'`
        inputfile=../REGRESSIONS/$stem.inp
        ../py -maxmem 250M -regression -notrace $inputfile
    fi
done

for f in *.ref
do
    # only true if *.ref isn't expanded because there is no matching file
    if [ -f $f ]
    then
        stem=`echo $f | sed -e 's/[.]ref$//'`
        inputfile=../EXAMPLES/$stem.inp
        ../py -maxmem 250M -regression -notrace $inputfile
    fi
done

for f in *.out
do
    # only true if *.ref isn't expanded because there is no matching file
    if [ -f $f ]
    then
        stem=`echo $f | sed -e 's/[.]out$//'`
        inputfile=../BEISPIEL/$stem.inp
        ../py -maxmem 250M -regression -notrace $inputfile
    fi
done
