#!/bin/bash
# -----------------------------------------------------------------------------
#
# Script for effectively testing Popeye using its example files on a multi-core
# Unix-ish machine.
#
# -----------------------------------------------------------------------------
#
# The functions in this file support parallel processing of commands (useful
# for multi-core computers).
# Note that the implementation is not perfect.  
#
# TODOs: 
#   - trap signals and cleanup control-directory
#
# Caveats:
#   - SIGKILLing the dispatcher leaves orphaned workers (but see note about
#      TMOUT below)
#
# -----------------------------------------------------------------------------
# **** Usage ****
# 
# Instead of:
# 
#   for item in "$@"; do
#     doSomething "$item"
#   done
# 
# code:
# 
#   source path-to-this-file
#   _cmd="doSomething"
# 
#   for item in "$@"; do
#     echo "$item"
#   done | dispatchWork
# 
# or:
# 
#   source path-to-this-file
#   dispatchWork -c "doSomething" "$@"
# 
# * Set DEBUG=1 to see what is going on.
# * Set PMAX=number of workers (defaults to number of processors in your system)
# * With TMOUT=n the workers time out after n seconds 
#   (TMOUT is a standard feature of read)
# 
# -----------------------------------------------------------------------------
# $Author: thomasmaeder $
# $Revision: 1.2 $
#
# License: GPL2
# -----------------------------------------------------------------------------

: ${DEBUG:=0}
: ${_cmd:=echo}
: ${PMAX:=`ls -1d /sys/devices/system/cpu/cpu* | wc -l`}

# Process a single item (internal function) -----------------------------------

processWorkItem() {
  [ $DEBUG -eq 1 ] && echo "info: worker ($1) - executing $_cmd $2" >&2
  eval $_cmd \"$2\"
}

# The worker-process (internal function) --------------------------------------

processWorkItems() {
  local line workerFifo="$1" dispatcherFifo="$2" id="$3" fd
  let fd=id+4
  exec 3<>"$dispatcherFifo"
  while ! echo "$id" >&3; do
    [ $DEBUG -eq 1 ] && \
        echo "warning: worker($id) - could not write idle-state" >&2
    sleep 1
  done
  [ $DEBUG -eq 1 ] && \
      echo "info: worker($id) - wrote idle-state" >&2
  while true; do 
    [ $DEBUG -eq 1 ] && echo "info: worker($id) - waiting for work" >&2
    read -r -u $fd line
    if [ $? -ne 0 ]; then
      [ $DEBUG -eq 1 ] && echo "info: worker($id) - received eof" >&2
      break
    fi
    [ $DEBUG -eq 1 ] && echo "info: worker($id) - processing: \"$line\"" >&2
    if [ "$line" = "EOF" ]; then
      break
    else
      processWorkItem "$id" "$line"
      while ! echo "$id" >&3; do
        [ $DEBUG -eq 1 ] && \
            echo "warning: worker($id) - could not write idle-state" >&2
        sleep 1
      done
      [ $DEBUG -eq 1 ] && \
          echo "info: worker($id) - wrote idle-state" >&2
    fi
  done
  [ $DEBUG -eq 1 ] && echo "info: worker($id) - removing workerFifo" >&2
  rm -f "$workerFifo"
}

# Start all workers (internal function) ---------------------------------------

startWorker() {
  local i fd fifo
  for (( i=0; i<PMAX; ++i )); do
    [ $DEBUG -eq 1 ] && echo "info: dispatcher - starting worker $i" >&2
    workerFifo="$controlDir/worker-$i"
    mkfifo "$workerFifo"
    let fd=i+4
    eval exec $fd\<\> "$workerFifo"
    processWorkItems "$workerFifo" "$dispatcherFifo" "$i" &
    CPID=$!
    [ $DEBUG -eq 1 ] && echo "info: dispatcher - PID of worker($i): $CPID" >&2
    cpidList="$cpidList $CPID"
  done
}

# Stop all workers (internal function) ----------------------------------------

stopWorker() {
  local i fifo
  for (( i=0; i<PMAX; ++i )); do
    fifo="$controlDir/worker-$i"
    [ $DEBUG -eq 1 ] && echo "info: dispatcher - stopping worker $i" >&2
    echo "EOF" > "$fifo"
  done
  wait $cpidList
}

# The dispatcher --------------------------------------------------------------

dispatchWork() {
  local OPTIND=1
  while getopts ":c:t:" opt; do
    case $opt in
      c) _cmd="$OPTARG";;
      t) TMOUT="$OPTARG";;
    esac
  done
  shift $((OPTIND-1))

  local idleId dispatcherFifo  controlDir=`mktemp -d`

  # create control-directory and start worker-processes

  [ $DEBUG -eq 1 ] && echo "info: dispatcher - created control-dir $controlDir" >&2
  dispatcherFifo="$controlDir/dispatcher"
  mkfifo "$dispatcherFifo"
  exec 3<>"$dispatcherFifo"

  startWorker

  # main loop: read and dispatch arguments

  if tty -s; then
    for line in "$@"; do
     [ $DEBUG -eq 1 ] && echo "info: dispatcher - processing: \"$line\"" >&2
     read -u 3 idleId
     [ $DEBUG -eq 1 ] && \
         echo "info: dispatcher - dispatching \"$line\" to worker($idleId)" >&2
     echo "$line" >> "$controlDir/worker-$idleId"
    done
  else
    while read -r -u 0 line; do
     [ $DEBUG -eq 1 ] && echo "info: dispatcher - processing: \"$line\"" >&2
     read -u 3 idleId
     [ $DEBUG -eq 1 ] && \
         echo "info: dispatcher - dispatching \"$line\" to worker($idleId)" >&2
     echo "$line" >> "$controlDir/worker-$idleId"
    done
  fi

  # stop worker-processes and remove control-directory

  stopWorker

  [ $DEBUG -eq 1 ] && echo "info: dispatcher - removing dispatcherFifo" >&2
  rm -f "$dispatcherFifo"
  [ $DEBUG -eq 1 ] && echo "info: dispatcher - removing control-dir $controlDir" >&2
  rm -fr "$controlDir"
}

###############################
# Popeye specific adaptations #
###############################

_cmd="../py -maxmem 1G -regression -notrace"
PMAX=2

for item in ../REGRESSIONS/*.inp ../EXAMPLES/*inp ../BEISPIEL/*inp; do
  echo "$item"
done | dispatchWork
