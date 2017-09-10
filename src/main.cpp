#include <iostream>

#include "tic-tac-toe.h"
#include "minimax.h"
#include "alpha-beta.h"

void print(const TicTacToe::StateType& state);
TicTacToe::ActionType getHumanAction(const TicTacToe::StateType& state);

int main()
{
    TicTacToe game;
    AlphaBeta<TicTacToe> search{game};
    TicTacToe::StateType state;
    print(state);
    while (!game.isTerminal(state))
    {
        auto action = state.player == 1 ? search.searchMax(state) :
                                          search.searchMin(state);
        std::cout << "(" << action.first << ", " << action.second << ")"
                  << std::endl;
        state = game.getResult(state, action);
        print(state);
    }
    std::cout << game.getUtility(state) << std::endl;
    return 0;
}

void print(const TicTacToe::StateType& state)
{
    std::cout << "-----" << std::endl;
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
    std::cout << "-----" << std::endl;
}

TicTacToe::ActionType getHumanAction(const TicTacToe::StateType& state)
{
    std::cout << "Play > ";
    size_t x, y;
    std::cin >> x >> y;
    while (!std::cin || x >= 3 || y >= 3 || state.board[x][y] != 0)
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Try again > ";
        std::cin >> x >> y;
    }
    return std::make_pair(x, y);
}
