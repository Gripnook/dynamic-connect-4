#pragma once

#include <limits>
#include <algorithm>
#include <utility>
#include <functional>

namespace Search {

// A class implementing the minimax search algorithm for a game of type Game.
//
// Game must define:
//      StateType - The type of the state representation for a position.
//      ActionType - The type of an action in the game.
//      EvalType - The type of a numerical position evaluation.
//
//      std::vector<ActionType> getActions(StateType)
//          A method to get a vector with the possible actions
//          that can be taken from a given state.
//
//      StateType getResult(StateType, ActionType)
//          A method to apply an action to a state and get the next state.
//
//      bool isTerminal(StateType)
//          A method to check if a state is terminal.
//
//      EvalType getUtility(StateType)
//          A method to get the utility value of a given terminal state.
template <typename Game>
class Minimax
{
public:
    using StateType = typename Game::StateType;
    using ActionType = typename Game::ActionType;
    using EvalType = typename Game::EvalType;

    using Heuristic = std::function<EvalType(const StateType&)>;

    Minimax(Game& game) : game(game)
    {
    }

    ActionType
        search(const StateType& state, Heuristic heuristic, int depth, bool isMax)
    {
        count = 1;
        this->heuristic = heuristic;

        auto init = isMax ? std::numeric_limits<EvalType>::lowest() :
                            std::numeric_limits<EvalType>::max();
        std::function<bool(EvalType, EvalType)> comp;
        if (isMax)
            comp = std::greater<EvalType>{};
        else
            comp = std::less<EvalType>{};

        auto actions = game.getActions(state);
        auto bestAction = std::make_pair(actions.front(), init);
        for (const auto& action : actions)
        {
            auto value =
                minimax(game.getResult(state, action), depth - 1, !isMax);
            if (comp(value, bestAction.second))
                bestAction = std::make_pair(action, value);
        }
        this->depth = depth;
        return bestAction.first;
    }

    int getLastCount() const
    {
        return count;
    }

    int getLastDepth() const
    {
        return depth;
    }

private:
    Game& game;
    int depth{};
    int count{0};
    Heuristic heuristic;

    EvalType minimax(const StateType& state, int depth, bool isMax)
    {
        ++count;
        if (game.isTerminal(state))
            return game.getUtility(state);
        else if (depth == 0)
            return heuristic(state);

        auto init = isMax ? std::numeric_limits<EvalType>::lowest() :
                            std::numeric_limits<EvalType>::max();
        std::function<bool(EvalType, EvalType)> comp;
        if (isMax)
            comp = std::greater<EvalType>{};
        else
            comp = std::less<EvalType>{};

        auto bestValue = init;
        for (const auto& action : game.getActions(state))
        {
            auto value =
                minimax(game.getResult(state, action), depth - 1, !isMax);
            if (comp(value, bestValue))
                bestValue = value;
        }
        return bestValue;
    }
};
}
