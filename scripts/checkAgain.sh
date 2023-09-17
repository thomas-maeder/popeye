#! /bin/bash

# Check again the example files that produced the output files
# currently in the current directy.
#
# Usage: checkAgain.sh
#
# Start from the directory containing the output files of a regression test.
#
# Uses: sed

die() { echo "$*" >&2; exit 2; }  # complain to STDERR and exit with error
needs_arg() { if [ -z "$OPTARG" ]; then die "No arg for --$OPT option"; fi; }

CORES=*3/4

# cf. https://stackoverflow.com/questions/402377/using-getopts-to-process-long-and-short-command-line-options
while getopts c:-: OPT; do
  # support long options: https://stackoverflow.com/a/28466267/519360
  if [ "$OPT" = "-" ]; then   # long option: reformulate OPT and OPTARG
    OPT="${OPTARG%%=*}"       # extract long option name
    OPTARG="${OPTARG#$OPT}"   # extract long option argument (may be empty)
    OPTARG="${OPTARG#=}"      # if long option argument, remove assigning `=`
  fi
  case "$OPT" in
    c | cores )    needs_arg; CORES="$OPTARG" ;;
    ??* )          die "Illegal option --$OPT" ;;  # bad long option
    ? )            exit 2 ;;  # bad short option (error reported via getopts)
  esac
done
shift $((OPTIND-1)) # remove parsed options and args from $@ list


SCRIPTDIR=$(dirname $0)
POPEYEDIR=${SCRIPTDIR}/..

. ${SCRIPTDIR}/parallelTester.lib

# command to be invoked in parallel
_cmd="${POPEYEDIR}/py -maxmem 1G -maxtrace 0 -regression"
#_cmd="WINEPREFIX=~/.wine ${POPEYEDIR}/pywin32.exe -maxmem 1G -maxtrace 0 -regression"
#_cmd="WINEPREFIX=~/.wine64 ${POPEYEDIR}/pywin64.exe -maxmem 1G -maxtrace 0 -regression"

# number of processors
CORES_AVAILABLE=$(nproc --all)
CORES_REQUESTED=$((($CORES_AVAILABLE$CORES)%$CORES_AVAILABLE))

if [ $CORES_REQUESTED -eq 0 ]
then
        echo "0 cores requested" >&2
        exit 1
fi

PMAX=$CORES_REQUESTED

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
