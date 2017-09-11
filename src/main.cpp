#include <iostream>
#include <chrono>

#include "dynamic-connect-4.h"
#include "iterative-alpha-beta.h"
#include "heuristics.h"

using Game = DynamicConnect4;

void print(const Game::StateType& state);

int main()
{
    Game game{Heuristic1{}};
    IterativeAlphaBeta<Game> search{game, 1000};
    Game::StateType state;
    print(state);
    while (!game.isTerminal(state))
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        if (state.player == 1)
        {
            game.setHeuristic(Heuristic1{});
            auto action = search.searchMax(state);
            state = game.getResult(state, action);
        }
        else
        {
            game.setHeuristic(Heuristic2{});
            auto action = search.searchMin(state);
            state = game.getResult(state, action);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        print(state);
        std::cout << (ms / 1000.0) << " seconds" << std::endl;
        std::cout << search.getLastCount() << " nodes searched" << std::endl;
    }
    std::cout << game.getUtility(state) << std::endl;
    return 0;
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
