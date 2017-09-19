#!/bin/bash

if [[ -z "$1" || -z "$2" ]]; then
    exit 1
fi

gameId="$1"
player="$2"

pipe="pipe$player"
logfile="logs/$gameId.$player.`date +%Y-%m-%dT%H:%M:%S%z`.log"

mkdir -p logs
rm -f --interactive=never "$pipe"
if mkfifo "$pipe"; then
    (./agent.exe -n -i$gameId -p$player -t19500 <"$pipe" \
        | tee >(telnet localhost 12345 >"$pipe"))        \
        &> >(tee -a "$logfile")
    rm --interactive=never "$pipe"
fi
