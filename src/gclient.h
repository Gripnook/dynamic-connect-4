#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>

#include "game/game.h"
#include "game/heuristics.h"
#include "search/iterative-alpha-beta.h"

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
    TelnetClient(int player, int timeLimitInMs)
        : player{player}, search{game, timeLimitInMs}
    {
        std::string login =
            "game.ai " + std::string(player == 1 ? "white" : "black");
        std::cout << login << std::endl;

        do
        {
            std::getline(std::cin, response);
            std::cerr << "Response: " << response << std::endl;
        } while (response != login);
    }

    void play()
    {
        printState();
        while (!game.isTerminal(state))
        {
            if (state.isPlayerOne)
            {
                if (player == 1)
                {
                    action = search.search(state, heuristic, true);
                    send();
                }
                else
                {
                    action = receive();
                }
            }
            else
            {
                if (player == 2)
                {
                    action = search.search(state, heuristic, false);
                    send();
                }
                else
                {
                    action = receive();
                }
            }
            state = game.getResult(state, action);
            printState();
        }
    }

private:
    int player;

    Game game;
    IterativeAlphaBeta<Game> search;
    StateType state;
    ActionType action;
    Heuristic<ConnectedElements, CentralDominance> heuristic{1.0f, 1.0f};

    std::string response;

    void send()
    {
        std::cout << to_string(action) << std::endl;
        std::getline(std::cin, response);
        assert(response == to_string(action));
    }

    ActionType receive()
    {
        char xc, yc, dirc;
        int x, y;
        Direction dir;
        auto actions = game.getActions(state);
        std::cin >> xc >> yc >> dirc;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!std::cin)
        {
            std::cin.clear();
            throw std::runtime_error("invalid input");
        }

        x = xc - '1';
        y = yc - '1';
        switch (dirc)
        {
        case 'E':
            dir = Direction::east;
            break;
        case 'W':
            dir = Direction::west;
            break;
        case 'S':
            dir = Direction::south;
            break;
        case 'N':
            dir = Direction::north;
            break;
        default:
            throw std::runtime_error("invalid direction");
        }

        if (std::find(
                std::begin(actions),
                std::end(actions),
                std::make_pair(Point{x, y}, dir)) == std::end(actions))
            throw std::runtime_error("invalid move");

        return ActionType{Point{x, y}, dir};
    }

    void printState()
    {
        std::cerr << "  1 2 3 4 5 6 7" << std::endl;
        for (int y = 0; y < boardSize; ++y)
        {
            std::cerr << (y + 1) << " ";
            for (int x = 0; x < boardSize - 1; ++x)
            {
                printPiece(x, y);
                std::cerr << ",";
            }
            printPiece(boardSize - 1, y);
            std::cerr << std::endl;
        }
    }

    void printPiece(int x, int y)
    {
        if (std::find(
                std::begin(state.whitePieces),
                std::end(state.whitePieces),
                Point{x, y}) != std::end(state.whitePieces))
            std::cerr << "O";
        else if (
            std::find(
                std::begin(state.blackPieces),
                std::end(state.blackPieces),
                Point{x, y}) != std::end(state.blackPieces))
            std::cerr << "X";
        else
            std::cerr << " ";
    }
};
