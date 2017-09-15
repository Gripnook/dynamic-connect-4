#!/bin/bash

if [[ -z "$1" ]]; then
    exit 1
fi

player="$1"
if [[ $player != "1" && $player != "2" ]]; then
    exit 2
fi

pipe="pipe$player"
logfile="logs/game.$player.`date +%Y-%m-%dT%H:%M:%S%z`.log"

mkdir -p logs
rm -f --interactive=never $pipe
if mkfifo $pipe; then
    ./agent.exe -s -p$player -t18000 <$pipe 2>(tee >(cat) >(cat >>"$logfile")) \
        | tee >(telnet localhost 12345 >$pipe) >(cat) >(cat >>"$logfile")
    rm --interactive=never $pipe
fi
