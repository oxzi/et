#!/bin/sh
# Requests the status from each et-process at an interval. If multiple
# et-processes are running, et-loop will run until the last one finishes.

SLEEP_ARG=$1
SLEEP_DEFAULT="1m"

SLEEP_VALUE="${SLEEP_ARG:-$SLEEP_DEFAULT}"

if pgrep -u "$(whoami)" -x et > /dev/null
then
  while [ et-status ]
  do
    et-status
    sleep $SLEEP_VALUE
  done
fi
