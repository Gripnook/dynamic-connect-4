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
    using Heuristic = std::function<Eval(const State&)>;

    IterativeAlphaBeta(Game& game, int timeLimitInMs)
        : game(game), timeLimitInMs{timeLimitInMs}
    {
    }

    Action searchMax(const State& state, Heuristic heuristic)
    {
        count = 1;
        this->heuristic = heuristic;
        startTime = std::chrono::high_resolution_clock::now();

        auto actions = game.getActions(state);
        std::map<Action, Eval> values;

        for (int depth = 2;; depth += 2)
        {
            maxDepth = depth;

            auto alpha = std::numeric_limits<Eval>::lowest();
            auto beta = std::numeric_limits<Eval>::max();

            for (const auto& action : actions)
            {
                auto value =
                    minValue(game.getResult(state, action), alpha, beta, 1);
                if (value == std::numeric_limits<Eval>::max())
                {
                    // We won.
                    this->depth = depth;
                    return action;
                }
                values[action] = value;
                alpha = std::max(alpha, value);
            }

            if (isTimeUp())
            {
                this->depth = depth - 2;
                return actions.front(); // The previous best action.
            }

            std::sort(
                std::begin(actions),
                std::end(actions),
                [&](const Action& lhs, const Action& rhs) {
                    return values[lhs] > values[rhs];
                });

            if (values[actions.front()] == std::numeric_limits<Eval>::lowest())
            {
                // Accept defeat.
                this->depth = depth;
                return actions.front();
            }
        }
    }

    Action searchMin(const State& state, Heuristic heuristic)
    {
        count = 1;
        this->heuristic = heuristic;
        startTime = std::chrono::high_resolution_clock::now();

        auto actions = game.getActions(state);
        std::map<Action, Eval> values;

        for (int depth = 2;; depth += 2)
        {
            maxDepth = depth;

            auto alpha = std::numeric_limits<Eval>::lowest();
            auto beta = std::numeric_limits<Eval>::max();

            for (const auto& action : actions)
            {
                auto value =
                    maxValue(game.getResult(state, action), alpha, beta, 1);
                if (value == std::numeric_limits<Eval>::lowest())
                {
                    // We won.
                    this->depth = depth;
                    return action;
                }
                values[action] = value;
                beta = std::min(beta, value);
            }

            if (isTimeUp())
            {
                this->depth = depth - 2;
                return actions.front(); // The previous best action.
            }

            std::sort(
                std::begin(actions),
                std::end(actions),
                [&](const Action& lhs, const Action& rhs) {
                    return values[lhs] < values[rhs];
                });

            if (values[actions.front()] == std::numeric_limits<Eval>::max())
            {
                // Accept defeat.
                this->depth = depth;
                return actions.front();
            }
        }
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
    int maxDepth;
    int count{0};
    int depth{0};
    Heuristic heuristic;

    int timeLimitInMs;
    std::chrono::high_resolution_clock::time_point startTime;

    Eval maxValue(const State& state, Eval alpha, Eval beta, int depth)
    {
        ++count;
        if (game.isTerminal(state))
            return game.getUtility(state);
        else if (depth >= maxDepth || isTimeUp())
            return heuristic(state);

        auto value = std::numeric_limits<Eval>::lowest();
        auto actions = depth + 1 == maxDepth ?
            game.getActions(state) :
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

    Eval minValue(const State& state, Eval alpha, Eval beta, int depth)
    {
        ++count;
        if (game.isTerminal(state))
            return game.getUtility(state);
        else if (depth >= maxDepth || isTimeUp())
            return heuristic(state);

        auto value = std::numeric_limits<Eval>::max();
        auto actions = depth + 1 == maxDepth ?
            game.getActions(state) :
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
            values[action] = heuristic(game.getResult(state, action));
        std::sort(
            std::begin(actions),
            std::end(actions),
            [&](const Action& lhs, const Action& rhs) {
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
