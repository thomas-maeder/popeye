#!/bin/bash

# Compare output files to the result of applying checkOuptut.sh to them
# Print names of output files where there is a difference.
#
# Run from the directory that is to contain the Popeye output.
#
# Pass -all as only option to test all output files

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
_cmd="${POPEYEDIR}/scripts/checkOutput.tcl"


# number of processors
CORES_AVAILABLE=$(nproc --all)
CORES_REQUESTED=$((($CORES_AVAILABLE$CORES)%$CORES_AVAILABLE))

if [ $CORES_REQUESTED -eq 0 ]
then
        echo "0 cores requested" >&2
        exit 1
fi

PMAX=$CORES_REQUESTED

(
    echo "REGRESSIONTESTS/*.out"
    echo "REGRESSIONTESTS/*.ref"
    echo "REGRESSIONTESTS/*.reg"
    echo "REGRESSIONTESTS/*.tst"
) | dispatchWork
