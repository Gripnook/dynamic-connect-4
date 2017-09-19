#include <iostream>
#include <chrono>
#include <string>
#include <algorithm>

#include "args.h"

#include "game.h"
#include "heuristics.h"
#include "minimax.h"
#include "alpha-beta.h"
#include "iterative-alpha-beta.h"

#include "gclient.h"

using namespace Args;
using namespace DynamicConnect4;
using namespace Search;

using StateType = Game::StateType;
using ActionType = Game::ActionType;
using EvalType = Game::EvalType;

void playGame(
    int humanPlayer, int timeLimitInMs, const StateType& initialState, bool debug);
ActionType getPlayerAction(const Game& game, const StateType& state);
void print(const StateType& state);

int main(int argc, char** argv)
{
    try
    {
        auto args = parse<Game>(argc, argv);
        if (args.telnet)
        {
            TelnetClient client{args.gameId,
                                args.player,
                                args.timeLimitInMs,
                                args.debug};
            client.play();
        }
        else
        {
            playGame(
                args.player, args.timeLimitInMs, args.initialState, args.debug);
        };
        return 0;
    }
    catch (ArgsError& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        printUsage<Game>(argv[0]);
        return 1;
    }
    catch (std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
    catch (...)
    {
        return 3;
    }
}

void playGame(
    int humanPlayer, int timeLimitInMs, const StateType& initialState, bool debug)
{
    Game game;
    IterativeAlphaBeta<Game> playerOneSearch{game, timeLimitInMs, debug};
    IterativeAlphaBeta<Game> playerTwoSearch{game, timeLimitInMs, debug};
    int playerOneWins = 0, playerTwoWins = 0, draws = 0;
    while (true)
    {
        StateType state = initialState;
        ActionType action;

        auto playerOneHeuristic =
            Heuristic<ConnectedPiecesV1, CentralDominanceV2>{1.0f, 1.0f};
        auto playerTwoHeuristic =
            Heuristic<ConnectedPiecesV4, CentralDominanceV2>{1.0f, 1.0f};

        print(state);
        std::cout << "player one evaluation: " << playerOneHeuristic(state)
                  << std::endl;
        std::cout << "player two evaluation: " << playerTwoHeuristic(state)
                  << std::endl;
        std::cout << std::endl;

        int move = 0;
        std::array<StateType, 5> previousStates;
        while (!game.isTerminal(state))
        {
            ++move;
            auto t1 = std::chrono::high_resolution_clock::now();
            if (state.isPlayerOne)
            {
                action = humanPlayer == 1 ?
                    getPlayerAction(game, state) :
                    playerOneSearch.search(state, playerOneHeuristic, true);
                state = game.getResult(state, action);
                print(state);
                std::cout << "move #" << move << std::endl;
                if (humanPlayer != 1)
                    std::cout << playerOneSearch.getLastCount()
                              << " nodes searched with max depth "
                              << playerOneSearch.getLastDepth() << std::endl;
            }
            else
            {
                action = humanPlayer == 2 ?
                    getPlayerAction(game, state) :
                    playerTwoSearch.search(state, playerTwoHeuristic, false);
                state = game.getResult(state, action);
                print(state);
                std::cout << "move #" << move << std::endl;
                if (humanPlayer != 2)
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
            std::cout << "action: " << action << std::endl;

            std::cout << "player one evaluation: " << playerOneHeuristic(state)
                      << std::endl;
            std::cout << "player two evaluation: " << playerTwoHeuristic(state)
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
        std::cout << "current score : " << playerOneWins << "-" << draws << "-"
                  << playerTwoWins << std::endl;
        std::cout
            << "============================================================"
            << std::endl;
    }
}

ActionType getPlayerAction(const Game& game, const StateType& state)
{
    ActionType action;
    auto actions = game.getActions(state);
    std::cout << "enter an action > ";
    while (true)
    {
        std::cin >> action;
        if (!std::cin)
            goto failure;

        if (std::find(std::begin(actions), std::end(actions), action) ==
            std::end(actions))
            goto failure;

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return action;

    failure:
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "invalid action. try again > ";
    }
}

void print(const StateType& state)
{
    std::cout << state;
}
