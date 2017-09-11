#pragma once

#include <limits>
#include <algorithm>
#include <utility>
#include <functional>

template <typename Game>
class Minimax
{
public:
    using State = typename Game::StateType;
    using Action = typename Game::ActionType;
    using Eval = typename Game::EvalType;
    using Heuristic = std::function<Eval(const State&)>;

    Minimax(Game& game, int maxDepth) : game(game), maxDepth{maxDepth}
    {
    }

    Action searchMax(const State& state, Heuristic heuristic)
    {
        count = 1;
        this->heuristic = heuristic;

        auto actions = game.getActions(state);
        auto bestAction =
            std::make_pair(actions.front(), std::numeric_limits<Eval>::lowest());
        for (const auto& action : actions)
        {
            auto value = minValue(game.getResult(state, action), 1);
            if (value > bestAction.second)
                bestAction = std::make_pair(action, value);
        }
        return bestAction.first;
    }

    Action searchMin(const State& state, Heuristic heuristic)
    {
        count = 1;
        this->heuristic = heuristic;

        auto actions = game.getActions(state);
        auto bestAction =
            std::make_pair(actions.front(), std::numeric_limits<Eval>::max());
        for (const auto& action : actions)
        {
            auto value = maxValue(game.getResult(state, action), 1);
            if (value < bestAction.second)
                bestAction = std::make_pair(action, value);
        }
        return bestAction.first;
    }

    Eval maxValue(const State& state, int depth)
    {
        ++count;
        if (game.isTerminal(state))
            return game.getUtility(state);
        else if (depth > maxDepth)
            return heuristic(state);
        auto value = std::numeric_limits<Eval>::lowest();
        for (const auto& action : game.getActions(state))
        {
            value = std::max(
                value, minValue(game.getResult(state, action), depth + 1));
        }
        return value;
    }

    Eval minValue(const State& state, int depth)
    {
        ++count;
        if (game.isTerminal(state))
            return game.getUtility(state);
        else if (depth > maxDepth)
            return heuristic(state);
        auto value = std::numeric_limits<Eval>::max();
        for (const auto& action : game.getActions(state))
        {
            value = std::min(
                value, maxValue(game.getResult(state, action), depth + 1));
        }
        return value;
    }

    int getLastCount() const
    {
        return count;
    }

private:
    Game& game;
    int maxDepth;
    int count{0};
    Heuristic heuristic;
};
