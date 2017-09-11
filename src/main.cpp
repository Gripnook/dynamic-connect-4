#include <iostream>
#include <chrono>

#include "tic-tac-toe.h"
#include "dynamic-connect-4.h"
#include "minimax.h"
#include "alpha-beta.h"

using Game = DynamicConnect4;

void print(const Game::StateType& state);

int main()
{
    Game game{6};
    AlphaBeta<Game> search{game};
    Game::StateType state;
    print(state);
    while (!game.isTerminal(state))
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        auto action = state.player == 1 ? search.searchMax(state) :
                                          search.searchMin(state);
        state = game.getResult(state, action);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        print(state);
        std::cout << ms / 1000.0 << " seconds" << std::endl;
    }
    std::cout << game.getUtility(state) << std::endl;
    return 0;
}

void print(const Game::StateType& state)
{
    std::cout << "---------" << std::endl;
    for (const auto& row : state.board)
    {
        std::cout << "|";
        for (const auto& col : row)
        {
            switch (col)
            {
            case 0:
                std::cout << " ";
                break;
            case 1:
                std::cout << "X";
                break;
            case 2:
                std::cout << "O";
                break;
            default:
                throw std::logic_error("impossible");
            }
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "---------" << std::endl;
}
