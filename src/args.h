#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

namespace Args {

template <typename Game>
struct Args
{
    bool telnet{false};
    std::string gameId;
    int player{0};
    int timeLimitInMs{20000};
    typename Game::StateType initialState;
    bool debug{false};
};

class ArgsError : public std::exception
{
public:
    ArgsError(std::string message) : message{message}
    {
    }

    virtual ~ArgsError() = default;

    virtual const char* what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message;
};

template <typename Game>
typename Game::StateType getState(const std::string& file);

template <typename Game>
void validate(const Args<Game>& args);

template <typename Game>
Args<Game> parse(int argc, char** argv)
{
    Args<Game> args;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg{argv[i]};
        if (arg.length() >= 2 && arg[0] == '-')
        {
            switch (arg[1])
            {
            case 'n':
            {
                args.telnet = true;
                break;
            }
            case 'i':
            {
                args.gameId = arg.substr(2);
                break;
            }
            case 'p':
            case 'h':
            {
                int player;
                std::stringstream ss{arg.substr(2)};
                ss >> player;
                if (!ss)
                    throw ArgsError{"invalid argument: " + arg};
                args.player = player;
                break;
            }
            case 't':
            {
                int timeLimitInMs;
                std::stringstream ss{arg.substr(2)};
                ss >> timeLimitInMs;
                if (!ss)
                    throw ArgsError{"invalid argument: " + arg};
                args.timeLimitInMs = timeLimitInMs;
                break;
            }
            case 'f':
            {
                std::string filename = arg.substr(2);
                args.initialState = getState<Game>(filename);
                break;
            }
            case 'd':
            {
                args.debug = true;
                break;
            }
            default:
            {
                throw ArgsError{"invalid argument: " + arg};
            }
            }
        }
        else
        {
            throw ArgsError{"invalid argument: " + arg};
        }
    }
    validate(args);
    return args;
}

template <typename Game>
typename Game::StateType getState(const std::string& file)
{
    typename Game::StateType state;
    std::ifstream in{file};
    if (!in)
        throw ArgsError{"file not found: " + file};
    in >> state;
    if (!in)
        throw ArgsError{"invalid initial state in " + file};
    return state;
}

template <typename Game>
void validate(const Args<Game>& args)
{
    if (args.player != 0 && args.player != 1 && args.player != 2)
        throw ArgsError{"invalid player: " + std::to_string(args.player)};

    if (args.timeLimitInMs < 0)
        throw ArgsError{"cannot play with negative time: " +
                        std::to_string(args.timeLimitInMs)};

    if (args.telnet)
    {
        if (args.gameId.empty())
            throw ArgsError{"invalid game id: \"" + args.gameId + "\""};
        for (auto ch : args.gameId)
            if (isspace(ch))
                throw ArgsError{"invalid game id: \"" + args.gameId + "\""};

        if (args.player == 0)
            throw ArgsError{"invalid player for telnet play: " +
                            std::to_string(args.player)};
    }
}

template <typename Game>
void printUsage(const std::string& progname)
{
    Args<Game> args;
    std::cerr
        << std::boolalpha << "Usage: " << progname
        << " [-n -i<id> -p<player>] [-t<ms>] [-d]" << std::endl
        << "       " << progname << " [-h<player>] [-f<filename>] [-t<ms>] [-d]"
        << std::endl
        << std::endl
        << "    -n:           "
           "Play the game using the telnet protocol through stdin "
           "and stdout. Defaults to "
        << args.telnet << "." << std::endl
        << "    -i<id>:       "
           "Specifies the id to use for the telnet game. <id> must not contain "
           "whitespace."
        << std::endl
        << "    -p<player>:   "
           "Specifies the player to play as for the telnet game. <player> = 1 "
           "or 2."
        << std::endl
        << std::endl
        << "    -h<player>:   "
           "Play as player <player> against the AI. <player> = 1 "
           "or 2, or 0 for AI vs AI play."
        << std::endl
        << "                  "
           "Defaults to "
        << args.player << "." << std::endl
        << "    -f<filename>: "
           "Load the initial state from the given filename."
        << std::endl
        << std::endl
        << "    -t<ms>:       "
           "Play with the specified time limit in ms. Defaults to "
        << args.timeLimitInMs << " ms." << std::endl
        << "    -d:           "
           "Play with additional debug information. Defaults to "
        << args.debug << "." << std::endl;
}
}
