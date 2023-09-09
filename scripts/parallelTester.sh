#!/bin/bash

# Run a regression test, using a configurable number of processors
#
# Run from the directory that is to contain the Popeye output.
#
# Options:
# --all also test the input files that take a loooooooooooong time.
# --cores=N indicate number of cores relative to the total number of cores
#           examples: +1 -1 *3/4

die() { echo "$*" >&2; exit 2; }  # complain to STDERR and exit with error
needs_arg() { if [ -z "$OPTARG" ]; then die "No arg for --$OPT option"; fi; }

CORES=*3/4

# cf. https://stackoverflow.com/questions/402377/using-getopts-to-process-long-and-short-command-line-options
while getopts ac:-: OPT; do
  # support long options: https://stackoverflow.com/a/28466267/519360
  if [ "$OPT" = "-" ]; then   # long option: reformulate OPT and OPTARG
    OPT="${OPTARG%%=*}"       # extract long option name
    OPTARG="${OPTARG#$OPT}"   # extract long option argument (may be empty)
    OPTARG="${OPTARG#=}"      # if long option argument, remove assigning `=`
  fi
  case "$OPT" in
    a | all )      ALL=true ;;
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
#_cmd="WINEPREFIX=~/.wine wine ${POPEYEDIR}/pywin32.exe -maxmem 1G -maxtrace 0 -regression"
#_cmd="WINEPREFIX=~/.wine64 wine64 ${POPEYEDIR}/pywin64.exe -maxmem 1G -maxtrace 0 -regression"

# number of processors
CORES_AVAILABLE=$(nproc --all)
CORES_REQUESTED=$((($CORES_AVAILABLE$CORES)%$CORES_AVAILABLE))

if [ $CORES_REQUESTED -eq 0 ]
then
	echo "0 cores requested" >&2
	exit 1
fi

PMAX=$CORES_REQUESTED

#DEBUG=1

# create and dispatch jobs
for item in ${POPEYEDIR}/TESTS/*.inp ${POPEYEDIR}/REGRESSIONS/*.inp ${POPEYEDIR}/EXAMPLES/*inp ${POPEYEDIR}/BEISPIEL/*inp; do
  echo "$item"
done | dispatchWork

if [ $ALL ]
then
  for item in ${POPEYEDIR}/EXAMPLES/lengthy/*inp ${POPEYEDIR}/BEISPIEL/lengthy/*inp; do
    echo "$item"
  done | dispatchWork
fi

