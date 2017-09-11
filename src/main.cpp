#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <utility>

#include "dynamic-connect-4.h"
#include "iterative-alpha-beta.h"
#include "heuristics.h"
#include "agent.h"

using Game = DynamicConnect4;

std::pair<int32_t, bool> parse(int argc, char** argv);
void playGame(int32_t timeLimitInMs);
void trainAgent(int32_t timeLimitInMs);
void print(const Game::StateType& state);

int main(int argc, char** argv)
{
    auto args = parse(argc, argv);
    if (args.second)
        trainAgent(args.first);
    else
        playGame(args.first);
    return 0;
}

std::pair<int32_t, bool> parse(int argc, char** argv)
{
    int32_t defaultTimeLimitInMs = 20000;
    bool defaultAgentTraining = false;
    if (argc > 1)
    {
        int32_t timeLimitInMs;
        std::stringstream ss{argv[1]};
        if (ss >> timeLimitInMs)
            defaultTimeLimitInMs = timeLimitInMs;
    }
    if (argc > 2)
    {
        bool agentTraining;
        std::stringstream ss{argv[2]};
        if (ss >> agentTraining)
            defaultAgentTraining = agentTraining;
    }
    return std::make_pair(defaultTimeLimitInMs, defaultAgentTraining);
}

void playGame(int32_t timeLimitInMs)
{
    auto heuristic =
        Heuristic<ConsecutiveElements, NearbyElements, Proximity, CentralDomination>{
            1.0, 1.0, 1.0, 1.0};
    Game game;
    IterativeAlphaBeta<Game> search{game, timeLimitInMs};
    Game::StateType state;
    print(state);
    while (!game.isTerminal(state))
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        if (state.player == 1)
        {
            auto action = search.searchMax(state, heuristic);
            state = game.getResult(state, action);
        }
        else
        {
            auto action = search.searchMin(state, heuristic);
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

void trainAgent(int32_t timeLimitInMs)
{
    Agent agent;
    auto heuristic =
        Heuristic<ConsecutiveElements, NearbyElements, Proximity, CentralDomination>{
            1.0, 1.0, 1.0, 1.0};
    int32_t player = 1;
    while (true)
    {
        agent.startGame(player);

        Game game;
        IterativeAlphaBeta<Game> search{game, timeLimitInMs};
        Game::StateType state;
        int32_t moveCount = 0;
        while (!game.isTerminal(state))
        {
            ++moveCount;
            if (moveCount > 100)
                goto exit;
            if (state.player == 1)
            {
                auto action = search.searchMax(
                    state, player == 1 ? agent.getHeuristic() : heuristic);
                state = game.getResult(state, action);
            }
            else
            {
                auto action = search.searchMin(
                    state, player == 2 ? agent.getHeuristic() : heuristic);
                state = game.getResult(state, action);
            }
        }
    exit:
        agent.endGame(game.getUtility(state), moveCount);
        player = player == 1 ? 2 : 1;
    }
}

void print(const Game::StateType& state)
{
    std::cout << "---------" << std::endl;
    for (size_t y = 0; y < Game::boardSize; ++y)
    {
        std::cout << "|";
        for (size_t x = 0; x < Game::boardSize; ++x)
        {
            if (std::find(
                    std::begin(state.whitePieces),
                    std::end(state.whitePieces),
                    std::make_pair(x, y)) != std::end(state.whitePieces))
                std::cout << "O";
            else if (
                std::find(
                    std::begin(state.blackPieces),
                    std::end(state.blackPieces),
                    std::make_pair(x, y)) != std::end(state.blackPieces))
                std::cout << "X";
            else
                std::cout << " ";
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "---------" << std::endl;
}
