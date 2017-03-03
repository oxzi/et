#!/usr/bin/env bash
# Requests the status from each et-process.

pgrep -u `whoami` -x et | while read -r pid; do
  kill -SIGUSR1 $pid
done
