#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <sstream>
#include <utility>
#include <algorithm>

#include "game/game.h"
#include "game/heuristics.h"
#include "search/minimax.h"
#include "search/alpha-beta.h"
#include "search/iterative-alpha-beta.h"

#include "gclient.h"

using namespace DynamicConnect4;
using namespace Search;

using StateType = Game::StateType;
using ActionType = Game::ActionType;
using EvalType = Game::EvalType;

struct Args
{
    bool server{false};
    int timeLimitInMs{18000};
    int player{0};
};

Args parse(int argc, char** argv);

void playGame(int timeLimitInMs, int humanPlayer);
ActionType getPlayerAction(const Game& game, const StateType& state);
void print(const StateType& state);

std::istream& operator>>(std::istream& in, StateType& state);
StateType getState(const std::string& file);

int main(int argc, char** argv)
{
    auto args = parse(argc, argv);
    if (args.server)
    {
        TelnetClient client{args.player, args.timeLimitInMs};
        client.play();
    }
    else
    {
        playGame(args.player, args.timeLimitInMs);
    };
    return 0;
}

Args parse(int argc, char** argv)
{
    Args args;
    for (int i = 0; i < argc; ++i)
    {
        std::string arg{argv[i]};
        if (arg.length() >= 2 && arg[0] == '-')
            switch (arg[1])
            {
            case 's':
            {
                args.server = true;
                break;
            }
            case 'p':
            case 'h':
            {
                int player;
                std::stringstream ss{arg.substr(2)};
                ss >> player;
                if (!ss)
                    throw std::runtime_error("invalid argument");
                args.player = player;
                break;
            }
            case 't':
            {
                int timeLimitInMs;
                std::stringstream ss{arg.substr(2)};
                ss >> timeLimitInMs;
                if (!ss)
                    throw std::runtime_error("invalid argument");
                args.timeLimitInMs = timeLimitInMs;
                break;
            }
            }
    }
    if (args.player != 0 && args.player != 1 && args.player != 2)
        throw std::runtime_error("invalid player");
    if (args.server && args.player == 0)
        throw std::runtime_error("invalid player");
    if (args.timeLimitInMs < 0)
        throw std::runtime_error("cannot play with negative time");
    return args;
}

void playGame(int timeLimitInMs, int humanPlayer)
{
    Game game;
    IterativeAlphaBeta<Game> playerOneSearch{game, timeLimitInMs};
    IterativeAlphaBeta<Game> playerTwoSearch{game, timeLimitInMs};
    int playerOneWins = 0, playerTwoWins = 0, draws = 0;
    while (true)
    {
        StateType state;
        ActionType action;

        auto genericHeuristic =
            Heuristic<ConsecutiveElements, CentralDominance>{1.0f, 1.0f};

        auto earlyGameHeuristic =
            Heuristic<EarlyCentralDominance, EarlyBlocking<1>>{1.0f, 1.0f};
        auto endGameHeuristic =
            Heuristic<ConnectedElements, EarlyCentralDominance>{1.0f, 1.0f};

        print(state);
        std::cout << "EarlyGame Evaluation: " << earlyGameHeuristic(state)
                  << std::endl;
        std::cout << "EndGame Evaluation: " << endGameHeuristic(state)
                  << std::endl;
        std::cout << "Generic Evaluation: " << genericHeuristic(state)
                  << std::endl;
        std::cout << std::endl;

        std::array<StateType, 5> previousStates;
        int move = 0;
        while (!game.isTerminal(state))
        {
            ++move;
            auto t1 = std::chrono::high_resolution_clock::now();
            if (state.isPlayerOne)
            {
                if (move <= 16)
                {
                    action = humanPlayer == 1 ?
                        getPlayerAction(game, state) :
                        playerOneSearch.search(state, earlyGameHeuristic, true);
                }
                else
                {
                    action = humanPlayer == 1 ?
                        getPlayerAction(game, state) :
                        playerOneSearch.search(state, endGameHeuristic, true);
                }
                state = game.getResult(state, action);
                print(state);
                std::cout << "move #" << move << std::endl;
                std::cout << playerOneSearch.getLastCount()
                          << " nodes searched with max depth "
                          << playerOneSearch.getLastDepth() << std::endl;
            }
            else
            {
                if (move <= 16)
                {
                    action = humanPlayer == 2 ?
                        getPlayerAction(game, state) :
                        playerTwoSearch.search(state, genericHeuristic, false);
                }
                else
                {
                    action = humanPlayer == 2 ?
                        getPlayerAction(game, state) :
                        playerTwoSearch.search(state, genericHeuristic, false);
                }
                state = game.getResult(state, action);
                print(state);
                std::cout << "move #" << move << std::endl;
                std::cout << playerTwoSearch.getLastCount()
                          << " nodes searched with max depth "
                          << playerTwoSearch.getLastDepth() << std::endl;
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            auto ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)
                    .count();
            std::cout << "turn took " << (ms / 1000.0) << " seconds"
                      << std::endl;
            std::cout << "action: " << to_string(action) << std::endl;

            std::cout << "EarlyGame Evaluation: " << earlyGameHeuristic(state)
                      << std::endl;
            std::cout << "EndGame Evaluation: " << endGameHeuristic(state)
                      << std::endl;
            std::cout << "Generic Evaluation: " << genericHeuristic(state)
                      << std::endl;
            std::cout << std::endl;

            std::copy(
                std::begin(previousStates) + 1,
                std::end(previousStates),
                std::begin(previousStates));
            previousStates.back() = state;
            if (previousStates.front() == previousStates.back())
                break;
        }

        if (previousStates.front() == previousStates.back())
        {
            std::cout << "draw!" << std::endl;
            ++draws;
        }
        else if (game.getUtility(state) == std::numeric_limits<EvalType>::max())
        {
            std::cout << "player 1 wins!" << std::endl;
            ++playerOneWins;
        }
        else if (game.getUtility(state) == std::numeric_limits<EvalType>::lowest())
        {
            std::cout << "player 2 wins!" << std::endl;
            ++playerTwoWins;
        }

        std::cout
            << "============================================================"
            << std::endl;
        std::cout << "Current score : " << playerOneWins << "-" << draws << "-"
                  << playerTwoWins << std::endl;
        std::cout
            << "============================================================"
            << std::endl;
    }
}

ActionType getPlayerAction(const Game& game, const StateType& state)
{
    char xc, yc, dirc;
    int x, y;
    Direction dir;
    auto actions = game.getActions(state);
    std::cout << "Enter an action > ";
    while (true)
    {
        std::cin >> xc >> yc >> dirc;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!std::cin)
        {
            std::cin.clear();
            goto failure;
        }

        x = xc - '1';
        y = yc - '1';
        switch (dirc)
        {
        case 'E':
        case 'e':
            dir = Direction::east;
            break;
        case 'W':
        case 'w':
            dir = Direction::west;
            break;
        case 'S':
        case 's':
            dir = Direction::south;
            break;
        case 'N':
        case 'n':
            dir = Direction::north;
            break;
        default:
            goto failure;
        }

        if (std::find(
                std::begin(actions),
                std::end(actions),
                std::make_pair(Point{x, y}, dir)) == std::end(actions))
            goto failure;

        return std::make_pair(Point{x, y}, dir);

    failure:
        std::cout << "Invalid action. Try again > ";
    }
}

void print(const StateType& state)
{
    std::cout << "  1 2 3 4 5 6 7" << std::endl;
    for (int y = 0; y < boardSize; ++y)
    {
        std::cout << (y + 1) << " ";
        for (int x = 0; x < boardSize; ++x)
        {
            if (std::find(
                    std::begin(state.whitePieces),
                    std::end(state.whitePieces),
                    Point{x, y}) != std::end(state.whitePieces))
                std::cout << "O,";
            else if (
                std::find(
                    std::begin(state.blackPieces),
                    std::end(state.blackPieces),
                    Point{x, y}) != std::end(state.blackPieces))
                std::cout << "X,";
            else
                std::cout << " ,";
        }
        std::cout << std::endl;
    }
}

std::istream& operator>>(std::istream& in, StateType& state)
{
    std::vector<Point> whitePieces;
    std::vector<Point> blackPieces;

    int i = 0, j = 0;
    for (char ch; in.get(ch);)
    {
        if (ch == 'O')
            whitePieces.emplace_back(i++, j);
        else if (ch == 'X')
            blackPieces.emplace_back(i++, j);
        else if (ch == ' ')
            ++i;
        if (ch == '\n')
        {
            i = 0;
            ++j;
        }
    }

    std::sort(std::begin(whitePieces), std::end(whitePieces));
    std::copy(
        std::begin(whitePieces),
        std::end(whitePieces),
        std::begin(state.whitePieces));
    std::sort(std::begin(blackPieces), std::end(blackPieces));
    std::copy(
        std::begin(blackPieces),
        std::end(blackPieces),
        std::begin(state.blackPieces));
    state.isPlayerOne = true;

    return in;
}

StateType getState(const std::string& file)
{
    StateType state;
    std::ifstream in{file};
    if (!in)
        throw std::runtime_error{"file not found"};
    in >> state;
    return state;
}
