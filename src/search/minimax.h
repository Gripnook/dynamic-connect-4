#pragma once

#include <limits>
#include <algorithm>
#include <utility>
#include <functional>

namespace Search {

template <typename Game>
class Minimax
{
public:
    using StateType = typename Game::StateType;
    using ActionType = typename Game::ActionType;
    using EvalType = typename Game::EvalType;

    using Heuristic = std::function<EvalType(const StateType&)>;

    Minimax(Game& game, int maxDepth) : game(game), maxDepth{maxDepth}
    {
    }

    ActionType search(const StateType& state, Heuristic heuristic, bool isMax)
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
            auto value = minimax(game.getResult(state, action), 1, !isMax);
            if (comp(value, bestAction.second))
                bestAction = std::make_pair(action, value);
        }
        return bestAction.first;
    }

    int getLastCount() const
    {
        return count;
    }

    int getLastDepth() const
    {
        return maxDepth;
    }

private:
    EvalType minimax(const StateType& state, int depth, bool isMax)
    {
        ++count;
        if (game.isTerminal(state))
            return game.getUtility(state);
        else if (depth >= maxDepth)
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
                minimax(game.getResult(state, action), depth + 1, !isMax);
            if (comp(value, bestValue))
                bestValue = value;
        }
        return bestValue;
    }

    Game& game;
    int maxDepth;
    int count{0};
    Heuristic heuristic;
};
}
