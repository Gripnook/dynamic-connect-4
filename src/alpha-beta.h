#pragma once

#include <limits>
#include <algorithm>
#include <utility>
#include <map>

template <typename Game>
class AlphaBeta
{
public:
    using State = typename Game::StateType;
    using Action = typename Game::ActionType;
    using Eval = typename Game::EvalType;

    AlphaBeta(Game game) : game{game}
    {
    }

    Action searchMax(const State& state)
    {
        auto alpha = std::numeric_limits<Eval>::lowest();
        auto beta = std::numeric_limits<Eval>::max();

        auto actions = sortByMaxAction(game.getActions(state), state);
        if (actions.empty())
            throw std::runtime_error("no actions possible");
        auto bestAction =
            std::make_pair(actions.front(), std::numeric_limits<Eval>::lowest());
        for (const auto& action : actions)
        {
            auto value = minValue(game.getResult(state, action), alpha, beta, 1);
            alpha = std::max(alpha, value);
            if (value > bestAction.second)
                bestAction = std::make_pair(action, value);
        }
        return bestAction.first;
    }

    Action searchMin(const State& state)
    {
        auto alpha = std::numeric_limits<Eval>::lowest();
        auto beta = std::numeric_limits<Eval>::max();

        auto actions = sortByMinAction(game.getActions(state), state);
        if (actions.empty())
            throw std::runtime_error("no actions possible");
        auto bestAction =
            std::make_pair(actions.front(), std::numeric_limits<Eval>::max());
        for (const auto& action : actions)
        {
            auto value = maxValue(game.getResult(state, action), alpha, beta, 1);
            beta = std::min(beta, value);
            if (value < bestAction.second)
                bestAction = std::make_pair(action, value);
        }
        return bestAction.first;
    }

    Eval maxValue(const State& state, Eval alpha, Eval beta, size_t depth)
    {
        if (game.cutoffTest(state, depth))
            return game.eval(state);
        auto value = std::numeric_limits<Eval>::lowest();
        auto actions = sortByMaxAction(game.getActions(state), state);
        for (const auto& action : actions)
        {
            value = std::max(
                value,
                minValue(game.getResult(state, action), alpha, beta, depth + 1));
            if (value >= beta)
                return value;
            alpha = std::max(alpha, value);
        }
        return value;
    }

    Eval minValue(const State& state, Eval alpha, Eval beta, size_t depth)
    {
        if (game.cutoffTest(state, depth))
            return game.eval(state);
        auto value = std::numeric_limits<Eval>::max();
        auto actions = sortByMinAction(game.getActions(state), state);
        for (const auto& action : actions)
        {
            value = std::min(
                value,
                maxValue(game.getResult(state, action), alpha, beta, depth + 1));
            if (value <= alpha)
                return value;
            beta = std::min(beta, value);
        }
        return value;
    }

private:
    Game game;

    std::vector<Action>
        sortByMaxAction(std::vector<Action> actions, const State& state) const
    {
        std::map<Action, Eval> values;
        for (const auto& action : actions)
            values[action] = game.eval(game.getResult(state, action));
        std::sort(
            std::begin(actions),
            std::end(actions),
            [&](const auto& lhs, const auto& rhs) {
                return values[lhs] > values[rhs];
            });
        return actions;
    }

    std::vector<Action>
        sortByMinAction(std::vector<Action> actions, const State& state) const
    {
        std::map<Action, Eval> values;
        for (const auto& action : actions)
            values[action] = game.eval(game.getResult(state, action));
        std::sort(
            std::begin(actions),
            std::end(actions),
            [&](const auto& lhs, const auto& rhs) {
                return values[lhs] < values[rhs];
            });
        return actions;
    }
};
