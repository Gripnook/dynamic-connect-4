#pragma once

#include <map>
#include <limits>
#include <algorithm>
#include <utility>
#include <functional>

template <typename Game>
class AlphaBeta
{
public:
    using State = typename Game::StateType;
    using Action = typename Game::ActionType;
    using Eval = typename Game::EvalType;
    using Heuristic = std::function<Eval(const State&)>;

    AlphaBeta(Game& game, int maxDepth) : game{game}, maxDepth{maxDepth}
    {
    }

    Action searchMax(const State& state, Heuristic heuristic)
    {
        count = 1;
        this->heuristic = heuristic;

        auto alpha = std::numeric_limits<Eval>::lowest();
        auto beta = std::numeric_limits<Eval>::max();

        auto actions =
            heuristicSort(game.getActions(state), state, std::greater<Eval>{});
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

    Action searchMin(const State& state, Heuristic heuristic)
    {
        count = 1;
        this->heuristic = heuristic;

        auto alpha = std::numeric_limits<Eval>::lowest();
        auto beta = std::numeric_limits<Eval>::max();

        auto actions =
            heuristicSort(game.getActions(state), state, std::less<Eval>{});
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

    Eval maxValue(const State& state, Eval alpha, Eval beta, int depth)
    {
        ++count;
        if (game.isTerminal(state))
            return game.getUtility(state);
        else if (depth > maxDepth)
            return heuristic(state);

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

    Eval minValue(const State& state, Eval alpha, Eval beta, int depth)
    {
        ++count;
        if (game.isTerminal(state))
            return game.getUtility(state);
        else if (depth > maxDepth)
            return heuristic(state);

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

    int getLastCount() const
    {
        return count;
    }

private:
    Game& game;
    int maxDepth;
    int count{0};
    Heuristic heuristic;

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
};
