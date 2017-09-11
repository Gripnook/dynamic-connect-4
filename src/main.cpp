#include <iostream>
#include <chrono>
#include <string>
#include <sstream>
#include <utility>
#include <tuple>
#include <algorithm>

#include "dynamic-connect-4.h"
#include "heuristics.h"
#include "minimax.h"
#include "alpha-beta.h"
#include "iterative-alpha-beta.h"

using Game = DynamicConnect4;
using StateType = Game::StateType;
using ActionType = Game::ActionType;
using EvalType = Game::EvalType;
using Direction = Game::Direction;

std::pair<int, int> parse(int argc, char** argv);
void challenger(int timeLimitInMs);
void playGame(int timeLimitInMs, int humanPlayer);
ActionType getPlayerAction(const Game& game, const StateType& state);
void print(const StateType& state);

int main(int argc, char** argv)
{
    auto args = parse(argc, argv);
    challenger(args.first);
    // playGame(args.first, args.second);
    return 0;
}

std::pair<int, int> parse(int argc, char** argv)
{
    int timeLimitInMs = 20000;
    int humanPlayer = 0;
    if (argc > 1)
    {
        int temp;
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

void challenger(int timeLimitInMs)
{
    Game game;
    IterativeAlphaBeta<Game> search{game, timeLimitInMs};

    double alpha = 1.0, beta = 1.0, gamma = 1.0;
    double increment = 0.25;
    auto heuristic =
        Heuristic<ConsecutiveElements, Proximity, CentralDominance>{alpha,
                                                                    beta,
                                                                    gamma};

    for (int i = -2; i <= 2; ++i)
    {
        for (int j = -2; j <= 2; ++j)
        {
            auto c_heuristic =
                Heuristic<ConsecutiveElements, Proximity, CentralDominance>{
                    alpha, beta + i * increment, gamma + j * increment};
            std::array<int, 2> results{};
            for (int k = 1; k < 2; ++k)
            {
                StateType state;
                int moveCount = 0;
                while (!game.isTerminal(state))
                {
                    ++moveCount;
                    if (moveCount > 100)
                        break;
                    if (state.player == 1)
                    {
                        auto action = search.searchMax(
                            state, state.player == k ? heuristic : c_heuristic);
                        state = game.getResult(state, action);
                    }
                    else
                    {
                        auto action = search.searchMin(
                            state, state.player == k ? heuristic : c_heuristic);
                        state = game.getResult(state, action);
                    }
                }
                if (moveCount > 100)
                {
                    results[k - 1] = 0;
                }
                else if (
                    (k == 1 && game.getUtility(state) > 0) ||
                    (k == 2 && game.getUtility(state) < 0))
                {
                    results[k - 1] = 1;
                }
                else
                {
                    results[k - 1] = -1;
                }
            }

            auto result = results[0] + results[1];
            if (result > 0)
            {
                std::cout << "challenger " << i << ", " << j
                          << " defeated with results " << results[0] << " and "
                          << results[1] << std::endl;
            }
            else if (result == 0)
            {
                std::cout << "drew with challenger " << i << ", " << j
                          << " with results " << results[0] << " and "
                          << results[1] << std::endl;
            }
            else
            {
                std::cout << "lost to challenger " << i << ", " << j
                          << " with results " << results[0] << " and "
                          << results[1] << std::endl;
                std::cout << "new parameters: " << alpha << ", "
                          << beta + i * increment << ", "
                          << gamma + j * increment << std::endl;
                heuristic = c_heuristic;
            }
        }
    }
    std::cout << "finished" << std::endl;
}

void playGame(int timeLimitInMs, int humanPlayer)
{
    Game game;
    IterativeAlphaBeta<Game> search{game, timeLimitInMs};
    StateType state;
    auto heuristic1 =
        Heuristic<ConsecutiveElements, Proximity, CentralDominance>{1.0,
                                                                    1.0,
                                                                    1.0};
    auto heuristic2 =
        Heuristic<ConsecutiveElements, Proximity, CentralDominance>{1.0,
                                                                    1.0,
                                                                    1.0};
    print(state);
    while (!game.isTerminal(state))
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        if (state.player == 1)
        {
            auto action = humanPlayer == 1 ? getPlayerAction(game, state) :
                                             search.searchMax(state, heuristic1);
            state = game.getResult(state, action);
        }
        else
        {
            auto action = humanPlayer == 2 ? getPlayerAction(game, state) :
                                             search.searchMin(state, heuristic2);
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
                std::make_tuple(x, y, dir)) == std::end(actions))
            goto failure;

        return std::make_tuple(x, y, dir);

    failure:
        std::cout << "Invalid action. Try again > ";
    }
}

void print(const StateType& state)
{
    std::cout << "  1 2 3 4 5 6 7" << std::endl;
    for (int y = 0; y < Game::boardSize; ++y)
    {
        std::cout << (y + 1) << " ";
        for (int x = 0; x < Game::boardSize; ++x)
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
