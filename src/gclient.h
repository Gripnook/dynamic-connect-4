#pragma once

#include <iostream>
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>
#include <thread>

#include "game.h"
#include "heuristics.h"
#include "iterative-alpha-beta.h"

using namespace DynamicConnect4;
using namespace Search;

using StateType = Game::StateType;
using ActionType = Game::ActionType;
using EvalType = Game::EvalType;

// This implements a telnet client that communicates with a game server using
// standard input and output. Debug information is printed using std::cerr.
class TelnetClient
{
public:
    TelnetClient(
        const std::string& gameId,
        int player,
        int timeLimitInMs,
        bool debug = false)
        : player{player}, search{game, timeLimitInMs, debug}
    {
        std::string login = gameId + " " + (player == 1 ? "white" : "black");
        std::cerr << "Sending: " << login << std::endl;
        std::cout << login << std::endl;

        do
        {
            std::getline(std::cin, response);
            if (response == "")
                throw std::runtime_error{"connection closed"};
            std::cerr << "Response: " << response << std::endl;
        } while (response != login);
    }

    void play()
    {
        printState();
        while (!game.isTerminal(state))
        {
            ++move;
            auto t1 = std::chrono::high_resolution_clock::now();
            isOurTurn = state.isPlayerOne ? (player == 1) : (player == 2);
            getAction();
            state = game.getResult(state, action);
            auto t2 = std::chrono::high_resolution_clock::now();
            timeInMs =
                std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)
                    .count();
            printState();
            printTurn();
        }
        printWinner();
    }

private:
    int player{};

    Game game;
    IterativeAlphaBeta<Game> search;
    StateType state;
    ActionType action;
    int move{0};
    bool isOurTurn{};
    int timeInMs{};

    Heuristic<ConnectedPiecesV1, CentralDominanceV2> heuristic{1.0f, 1.0f};

    std::string response;

    void getAction()
    {
        if (isOurTurn)
        {
            // This reduces our memory footprint by ensuring the search
            // transposition table is only used by other threads.
            std::thread compute{[&]() {
                action =
                    search.search(state, heuristic, player == 1 ? true : false);
            }};
            compute.join();
            send();
        }
        else
        {
            std::thread compute{[&]() {
                search.search(state, heuristic, player == 1 ? false : true);
            }};
            action = receive();
            auto timeLimitInMs = search.stop();
            if (compute.joinable())
                compute.join();
            search.reset(timeLimitInMs);
        }
    }

    void send()
    {
        std::cerr << "Sending: " << action << std::endl;
        std::cout << action << std::endl;
        std::getline(std::cin, response);
        std::cerr << "Response: " << response << std::endl;
    }

    ActionType receive()
    {
        ActionType action;
        auto actions = game.getActions(state);
        std::getline(std::cin, response);
        if (!std::cin)
        {
            std::cin.clear();
            throw std::runtime_error{"invalid input"};
        }
        std::stringstream in{response};
        in >> action;
        if (!in)
            throw std::runtime_error{"invalid input: " + response};

        if (std::find(std::begin(actions), std::end(actions), action) ==
            std::end(actions))
        {
            std::stringstream ss;
            ss << action;
            throw std::runtime_error{"invalid move: " + ss.str()};
        }

        return action;
    }

    void printState()
    {
        std::cerr << state;
    }

    void printTurn()
    {
        std::cerr << "move #" << move << std::endl;
        if (isOurTurn)
            std::cerr << search.getLastCount()
                      << " nodes searched with max depth "
                      << search.getLastDepth() << std::endl;
        std::cerr << "turn took " << (timeInMs / 1000.0) << " seconds"
                  << std::endl;
        std::cerr << "action: " << action << std::endl;
        std::cerr << "position evaluation: " << heuristic(state) << std::endl;
        std::cerr << std::endl;
    }

    void printWinner()
    {
        if ((game.getUtility(state) > 0 && player == 1) ||
            (game.getUtility(state) < 0 && player == 2))
            std::cerr << "we won!" << std::endl;
        else
            std::cerr << "we lost!" << std::endl;
    }
};
