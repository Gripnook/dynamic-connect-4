# Dynamic Connect-4

An agent capable of playing the game of dynamic connect-4 designed for ECSE-526 Artificial Intelligence.

# Usage

To compile the agent program, run the `make` command from the top-level directory. This will generate the `agent.exe` program. Note that only the `g++` compiler is supported.

To run the agent program, execute `./agent.exe`. To play against the AI as player 1 or 2, use the `-h<player>` parameter. To load a custom initial state, use the `-f<filename>` parameter. To set the time limit, use the `-t<ms>` parameter. For a full list of possible parameters, use the `-H` flag. Note that any arguments to a parameter must immediately follow it with no spaces.

The `start-server.sh` and `start-agent.sh` script files have been included for server play:
- `start-server.sh`: Starts a telnet game server on port 12345 with a time limit of 20s per move.
- `start-agent.sh`: Starts the agent program in a telnet client configuration. This script will produce logs of each game in the `logs` directory. Note that the `telnet` program is required. The usage is `./start-agent.sh <server> <port> <game-id> <1|2>`.
