#include <iostream>
#include <chrono>
#include <string>
#include <sstream>
#include <utility>
#include <tuple>
#include <algorithm>

#include "dynamic-connect-4.h"
#include "iterative-alpha-beta.h"
#include "heuristics.h"

using Game = DynamicConnect4;

std::pair<int32_t, int32_t> parse(int argc, char** argv);
void playGame(int32_t timeLimitInMs, int32_t humanPlayer);
Game::ActionType getPlayerAction(const Game& game, const Game::StateType& state);
void print(const Game::StateType& state);

int main(int argc, char** argv)
{
    auto args = parse(argc, argv);
    playGame(args.first, args.second);
    return 0;
}

std::pair<int32_t, int32_t> parse(int argc, char** argv)
{
    int32_t timeLimitInMs = 20000;
    int32_t humanPlayer = 0;
    if (argc > 1)
    {
        int32_t temp;
        std::stringstream ss{argv[1]};
        if (ss >> temp)
            timeLimitInMs = temp;
    }
    if (argc > 2)
    {
        if (std::string{argv[2]} == "-h1")
            humanPlayer = 1;
        else if (std::string{argv[2]} == "-h2")
            humanPlayer = 2;
    }
    return std::make_pair(timeLimitInMs, humanPlayer);
}

void playGame(int32_t timeLimitInMs, int32_t humanPlayer)
{
    Game game;
    IterativeAlphaBeta<Game> search{game, timeLimitInMs};
    Game::StateType state;
    auto heuristic =
        Heuristic<ConsecutiveElements, Proximity, CentralDomination>{1.0,
                                                                     0.35,
                                                                     0.60};
    print(state);
    while (!game.isTerminal(state))
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        if (state.player == 1)
        {
            auto action = humanPlayer == 1 ? getPlayerAction(game, state) :
                                             search.searchMax(state, heuristic);
            state = game.getResult(state, action);
        }
        else
        {
            auto action = humanPlayer == 2 ? getPlayerAction(game, state) :
                                             search.searchMin(state, heuristic);
            state = game.getResult(state, action);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        print(state);
        std::cout << (ms / 1000.0) << " seconds" << std::endl;
        std::cout << search.getLastCount() << " nodes searched with max depth "
                  << search.getLastDepth() << std::endl;
    }
    std::cout << game.getUtility(state) << std::endl;
}

Game::ActionType getPlayerAction(const Game& game, const Game::StateType& state)
{
    char xc, yc, dirc;
    size_t x, y;
    Game::Direction dir;
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
            dir = Game::Direction::east;
            break;
        case 'W':
        case 'w':
            dir = Game::Direction::west;
            break;
        case 'S':
        case 's':
            dir = Game::Direction::south;
            break;
        case 'N':
        case 'n':
            dir = Game::Direction::north;
            break;
        default:
            goto failure;
        }

        if (std::find(
                std::begin(actions),
                std::end(actions),
                std::make_tuple(x, y, dir)) == std::end(actions))
            goto failure;

        return std::make_tuple(x, y, dir);

    failure:
        std::cout << "Invalid action. Try again > ";
    }
}

void print(const Game::StateType& state)
{
    std::cout << "  1 2 3 4 5 6 7" << std::endl;
    for (size_t y = 0; y < Game::boardSize; ++y)
    {
        std::cout << (y + 1) << " ";
        for (size_t x = 0; x < Game::boardSize; ++x)
        {
            if (std::find(
                    std::begin(state.whitePieces),
                    std::end(state.whitePieces),
                    std::make_pair(x, y)) != std::end(state.whitePieces))
                std::cout << "O,";
            else if (
                std::find(
                    std::begin(state.blackPieces),
                    std::end(state.blackPieces),
                    std::make_pair(x, y)) != std::end(state.blackPieces))
                std::cout << "X,";
            else
                std::cout << " ,";
        }
        std::cout << std::endl;
    }
}
