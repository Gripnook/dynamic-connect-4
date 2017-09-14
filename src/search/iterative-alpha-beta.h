#pragma once

#include <map>
#include <limits>
#include <algorithm>
#include <utility>
#include <functional>
#include <chrono>

#include "search/state-cache.h"

namespace Search {

template <typename Game>
class IterativeAlphaBeta
{
public:
    using StateType = typename Game::StateType;
    using ActionType = typename Game::ActionType;
    using EvalType = typename Game::EvalType;

    using Heuristic = std::function<EvalType(const StateType&)>;

    IterativeAlphaBeta(Game& game, int timeLimitInMs)
        : game(game), timeLimitInMs{timeLimitInMs}
    {
    }

    ActionType search(const StateType& state, Heuristic heuristic, bool isMax)
    {
        count = 1;
        this->heuristic = heuristic;
        startTime = std::chrono::high_resolution_clock::now();

        auto actions = game.getActions(state);
        std::map<ActionType, EvalType> values;

        auto winIndicator = isMax ? std::numeric_limits<EvalType>::max() :
                                    std::numeric_limits<EvalType>::lowest();
        auto lossIndicator = isMax ? std::numeric_limits<EvalType>::lowest() :
                                     std::numeric_limits<EvalType>::max();
        std::function<bool(EvalType, EvalType)> comp;
        if (isMax)
            comp = std::greater<EvalType>{};
        else
            comp = std::less<EvalType>{};

        for (int depth = 2;; depth += 2)
        {
            maxDepth = depth;
            cache.reset();

            auto alpha = std::numeric_limits<EvalType>::lowest();
            auto beta = std::numeric_limits<EvalType>::max();

            for (const auto& action : actions)
            {
                auto value = alphaBeta(
                    game.getResult(state, action), alpha, beta, 1, !isMax);
                if (value == winIndicator)
                {
                    // We found our goal, so we can stop searching.
                    this->depth = depth;
                    return action;
                }
                values[action] = value;
                if (isMax)
                    alpha = std::max(alpha, value);
                else
                    beta = std::min(beta, value);
            }

            // std::cout << "searched " << count << " nodes so far at depth "
            //           << depth << ", with " << cache.localCacheSize()
            //           << " nodes cached locally and " << cache.globalCacheSize()
            //           << " nodes cached globally" << std::endl;

            if (isTimeUp())
            {
                // We ran out of time, so return the previous best action.
                this->depth = depth - 2;
                return actions.front();
            }

            // Sort the actions so the best ones are first.
            actions = heuristicSort(actions, comp, values);

            // If either the best action is a loss, or the second best
            // action is a loss, we can safely pick the best action since it is
            // guaranteed that there is none that is better.
            if (values[actions.front()] == lossIndicator ||
                (actions.size() == 1 || values[actions[1]] == lossIndicator))
            {
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

    StateCache<Game> cache{[](const StateType& /*state*/, const EvalType& value) {
        // We cache winning and losing positions globally.
        return value == std::numeric_limits<EvalType>::max() ||
            value == std::numeric_limits<EvalType>::lowest();
    }};

    int timeLimitInMs;
    std::chrono::high_resolution_clock::time_point startTime;

    EvalType alphaBeta(
        const StateType& state,
        EvalType alpha,
        EvalType beta,
        int depth,
        bool isMax)
    {
        ++count;
        if (game.isTerminal(state))
            return game.getUtility(state);
        else if (depth >= maxDepth || isTimeUp())
            return heuristic(state);

        if (cache.contains(state))
            return cache.get(state);

        auto init = isMax ? std::numeric_limits<EvalType>::lowest() :
                            std::numeric_limits<EvalType>::max();
        std::function<bool(EvalType, EvalType)> comp;
        if (isMax)
            comp = std::greater<EvalType>{};
        else
            comp = std::less<EvalType>{};

        auto bestValue = init;
        auto actions = game.getActions(state);
        // Sorting the actions using the heuristic helps us consider
        // the best actions first.
        if (depth + 1 < maxDepth)
            actions = heuristicSort(actions, state, comp);
        for (const auto& action : actions)
        {
            auto value = alphaBeta(
                game.getResult(state, action), alpha, beta, depth + 1, !isMax);
            if (isMax)
            {
                bestValue = std::max(bestValue, value);
                alpha = std::max(alpha, bestValue);
            }
            else
            {
                bestValue = std::min(bestValue, value);
                beta = std::min(beta, bestValue);
            }
            if (alpha >= beta)
                break;
        }
        cache.set(state, bestValue);
        return bestValue;
    }

    std::vector<ActionType> heuristicSort(
        std::vector<ActionType>& actions,
        const StateType& state,
        std::function<bool(EvalType, EvalType)> comp) const
    {
        std::map<ActionType, EvalType> values;
        for (const auto& action : actions)
            values[action] = heuristic(game.getResult(state, action));
        return heuristicSort(actions, comp, values);
    }

    std::vector<ActionType> heuristicSort(
        std::vector<ActionType>& actions,
        std::function<bool(EvalType, EvalType)> comp,
        const std::map<ActionType, EvalType>& values) const
    {
        std::sort(
            std::begin(actions),
            std::end(actions),
            [&](const ActionType& lhs, const ActionType& rhs) {
                return comp(values.find(lhs)->second, values.find(rhs)->second);
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
}
