#!/bin/bash

progname=`basename "$0"`

printUsage () {
    echo "Usage: $progname <server> <port> <game-id> <1|2>"
}

if [[ -z "$1" || -z "$2" || -z "$3" || -z "$4" ]]; then
    echo "Error: invalid argument"
    printUsage
    exit 1
fi

server="$1"
port="$2"
if ! echo q | telnet -eq "$server" "$port"; then
    echo "Error: server not found"
    printUsage
    exit 2
fi

gameId="$3"
if [[ "$gameId" =~ .*[[:space:]]+.* ]]; then
    echo "Error: game-id must not contain whitespace"
    printUsage
    exit 3
fi

player="$4"
if [[ "$player" != "1" && "$player" != "2" ]]; then
    echo "Error: invalid player"
    printUsage
    exit 4
fi

pipe="pipe$player"
logfile="logs/$gameId.$player.`date +%Y-%m-%dT%H:%M:%S%z`.log"

mkdir -p logs
rm -f --interactive=never "$pipe"
if mkfifo "$pipe"; then
    (./agent.exe -n -i"$gameId" -p"$player" -t19500 <"$pipe" \
        | tee >(telnet "$server" "$port" >"$pipe"))          \
        &> >(tee -a "$logfile")
    rm --interactive=never "$pipe"
fi
