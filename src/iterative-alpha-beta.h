#pragma once

#include <map>
#include <limits>
#include <algorithm>
#include <utility>
#include <functional>
#include <chrono>

template <typename Game>
class IterativeAlphaBeta
{
public:
    using State = typename Game::StateType;
    using Action = typename Game::ActionType;
    using Eval = typename Game::EvalType;

    IterativeAlphaBeta(Game& game, size_t timeLimitInMs)
        : game{game}, timeLimitInMs{timeLimitInMs}
    {
    }

    Action searchMax(const State& state)
    {
        count = 0;
        startTime = std::chrono::high_resolution_clock::now();

        auto actions = game.getActions(state);
        std::map<Action, Eval> values;

        for (size_t depth = 1;; ++depth)
        {
            maxDepth = depth;

            auto alpha = std::numeric_limits<Eval>::lowest();
            auto beta = std::numeric_limits<Eval>::max();

            for (const auto& action : actions)
            {
                auto value =
                    minValue(game.getResult(state, action), alpha, beta, 1);
                values[action] = value;
                alpha = std::max(alpha, value);
            }

            if (isTimeUp())
                return actions.front(); // The previous best action.

            std::sort(
                std::begin(actions),
                std::end(actions),
                [&](const auto& lhs, const auto& rhs) {
                    return values[lhs] > values[rhs];
                });

            if (values[actions.front()] == std::numeric_limits<Eval>::max())
                return actions.front();
        }
    }

    Action searchMin(const State& state)
    {
        count = 0;
        startTime = std::chrono::high_resolution_clock::now();

        auto actions = game.getActions(state);
        std::map<Action, Eval> values;

        for (size_t depth = 1;; ++depth)
        {
            maxDepth = depth;

            auto alpha = std::numeric_limits<Eval>::lowest();
            auto beta = std::numeric_limits<Eval>::max();

            for (const auto& action : actions)
            {
                auto value =
                    maxValue(game.getResult(state, action), alpha, beta, 1);
                values[action] = value;
                beta = std::min(beta, value);
            }

            if (isTimeUp())
                return actions.front(); // The previous best action.

            std::sort(
                std::begin(actions),
                std::end(actions),
                [&](const auto& lhs, const auto& rhs) {
                    return values[lhs] < values[rhs];
                });

            if (values[actions.front()] == std::numeric_limits<Eval>::lowest())
                return actions.front();
        }
    }

    size_t getLastCount() const
    {
        return count;
    }

private:
    Game& game;
    size_t maxDepth;
    size_t count{0};

    size_t timeLimitInMs;
    std::chrono::high_resolution_clock::time_point startTime;

    Eval maxValue(const State& state, Eval alpha, Eval beta, size_t depth)
    {
        ++count;
        if (game.isTerminal(state))
            return game.getUtility(state);
        else if (depth > maxDepth || isTimeUp())
            return game.eval(state);

        auto value = std::numeric_limits<Eval>::lowest();
        auto actions =
            heuristicSort(game.getActions(state), state, std::greater<Eval>{});
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
        ++count;
        if (game.isTerminal(state))
            return game.getUtility(state);
        else if (depth > maxDepth || isTimeUp())
            return game.eval(state);

        auto value = std::numeric_limits<Eval>::max();
        auto actions =
            heuristicSort(game.getActions(state), state, std::less<Eval>{});
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

    std::vector<Action> heuristicSort(
        std::vector<Action> actions,
        const State& state,
        std::function<bool(Eval, Eval)> comp) const
    {
        std::map<Action, Eval> values;
        for (const auto& action : actions)
            values[action] = game.eval(game.getResult(state, action));
        std::sort(
            std::begin(actions),
            std::end(actions),
            [&](const auto& lhs, const auto& rhs) {
                return comp(values[lhs], values[rhs]);
            });
        return actions;
    }

    bool isTimeUp() const
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto timeInMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now - startTime)
                            .count();
        return timeInMs >= timeLimitInMs;
    }
};
