#pragma once

#include <limits>
#include <algorithm>
#include <utility>

template <typename Game>
class Minimax
{
public:
    using State = typename Game::StateType;
    using Action = typename Game::ActionType;
    using Eval = typename Game::EvalType;

    Minimax(Game game) : game{game}
    {
    }

    Action searchMax(const State& state)
    {
        auto actions = game.getActions(state);
        if (actions.empty())
            throw std::runtime_error("no actions possible");
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

    Action searchMin(const State& state)
    {
        auto actions = game.getActions(state);
        if (actions.empty())
            throw std::runtime_error("no actions possible");
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

    Eval maxValue(const State& state, size_t depth)
    {
        if (game.cutoffTest(state, depth))
            return game.eval(state);
        auto value = std::numeric_limits<Eval>::lowest();
        for (const auto& action : game.getActions(state))
        {
            value = std::max(
                value, minValue(game.getResult(state, action), depth + 1));
        }
        return value;
    }

    Eval minValue(const State& state, size_t depth)
    {
        if (game.cutoffTest(state, depth))
            return game.eval(state);
        auto value = std::numeric_limits<Eval>::max();
        for (const auto& action : game.getActions(state))
        {
            value = std::min(
                value, maxValue(game.getResult(state, action), depth + 1));
        }
        return value;
    }

private:
    Game game;
};
