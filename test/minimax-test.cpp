#include "catch.hpp"

#include "search/minimax.h"
#include "tic-tac-toe.h"

using Game = TicTacToe;
using State = Game::StateType;
using Action = Game::ActionType;
using Eval = Game::EvalType;
using Heuristic = Minimax<Game>::Heuristic;

SCENARIO("Minimax works for tic-tac-toe")
{
    Game game;
    Heuristic heuristic = [&](const State& state) {
        return game.isTerminal(state) ? game.getUtility(state) : 0;
    };

    GIVEN("A winning position of tic-tac-toe for player 1")
    {
        State state;
        state.board = {{{1, 2, 2}, {0, 1, 0}, {0, 0, 0}}};

        WHEN("Player 1 performs minimax search with depth 1")
        {
            Minimax<Game> search{game, 1};
            Action action = search.searchMax(state, heuristic);
            THEN("The player finds the winning move")
            {
                REQUIRE(action == std::make_pair(2, 2));
            }
        }
    }

    GIVEN("A drawing position of tic-tac-toe for player 1")
    {
        State state;
        state.board = {{{1, 2, 1}, {0, 1, 0}, {2, 0, 2}}};

        WHEN("Player 1 performs minimax search with depth 1")
        {
            Minimax<Game> search{game, 1};
            Action action = search.searchMax(state, heuristic);
            THEN("The player finds the non-losing move")
            {
                REQUIRE(action == std::make_pair(2, 1));
            }
        }
    }
}
