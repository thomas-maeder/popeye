#! /bin/bash

# Check again the example files that produced the output files
# currently in the current directy.
#
# Usage: checkAgain.sh
#
# Start from the directory containing the output files of a regression test.
#
# Uses: sed

SCRIPTDIR=$(dirname $0)
POPEYEDIR=${SCRIPTDIR}/..

. ${SCRIPTDIR}/parallelTester.lib

# command to be invoked in parallel
_cmd="${POPEYEDIR}/py -maxmem 1G -maxtrace 0 -regression"
#_cmd="WINEPREFIX=~/.wine ${POPEYEDIR}/pywin32.exe -maxmem 1G -maxtrace 0 -regression"
#_cmd="WINEPREFIX=~/.wine64 ${POPEYEDIR}/pywin64.exe -maxmem 1G -maxtrace 0 -regression"

# number of processors
PMAX=3

(for f in *.tst
do
    # only true if *.tst isn't expanded because there is no matching file
    if [ -f $f ]
    then
        stem=`echo $f | sed -e 's/[.]tst$//'`
        echo ${POPEYEDIR}/TESTS/$stem.inp
    fi
done

for f in *.reg
do
    # only true if *.reg isn't expanded because there is no matching file
    if [ -f $f ]
    then
        stem=`echo $f | sed -e 's/[.]reg$//'`
        echo ${POPEYEDIR}/REGRESSIONS/$stem.inp
    fi
done

for f in *.ref
do
    # only true if *.ref isn't expanded because there is no matching file
    if [ -f $f ]
    then
        stem=`echo $f | sed -e 's/[.]ref$//'`
        echo ${POPEYEDIR}/EXAMPLES/$stem.inp
    fi
done

for f in *.out
do
    # only true if *.out isn't expanded because there is no matching file
    if [ -f $f ]
    then
        stem=`echo $f | sed -e 's/[.]out$//'`
        echo ${POPEYEDIR}/BEISPIEL/$stem.inp
    fi
done) | dispatchWork
