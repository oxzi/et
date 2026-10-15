#!/bin/sh
# Requests the status from each et-process.

pgrep -u "$(whoami)" -x et | while read -r pid; do
  kill -USR1 "$pid"
done
