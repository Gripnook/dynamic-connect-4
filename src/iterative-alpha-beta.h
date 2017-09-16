#pragma once

#include <map>
#include <limits>
#include <algorithm>
#include <utility>
#include <functional>
#include <chrono>
#include <iostream>

#include "transposition-table.h"

namespace Search {

template <typename Game>
class IterativeAlphaBeta
{
public:
    using StateType = typename Game::StateType;
    using ActionType = typename Game::ActionType;
    using EvalType = typename Game::EvalType;

    using Heuristic = std::function<EvalType(const StateType&)>;

    IterativeAlphaBeta(Game& game, int timeLimitInMs, bool debug = false)
        : game(game), timeLimitInMs{timeLimitInMs}, debug{debug}
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

        if (debug)
        {
            std::cerr << "========== actions ==========" << std::endl;
        }

        for (int depth = 1;; ++depth)
        {
            maxDepth = depth;
            transpositionTable.reset();

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

            if (debug)
            {
                std::cerr << "searched " << count << " nodes so far at depth "
                          << depth << ", with " << transpositionTable.size()
                          << " nodes cached" << std::endl;
            }

            if (isTimeUp())
            {
                // We ran out of time, so return the previous best action.
                this->depth = depth - 1;
                return actions.front();
            }

            // Sort the actions so the best ones are first.
            // Given a stable sort, this will also ensure that better actions at
            // a lower depth will be ahead of now equal valued actions.
            actions = heuristicSort(actions, comp, values);

            if (debug)
            {
                std::cerr << "depth " << depth << " => ";
                for (const auto& action : actions)
                    std::cerr << action << ": " << values[action] << "; ";
                std::cerr << std::endl;
            }

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
    int maxDepth{};
    int count{0};
    int depth{0};
    Heuristic heuristic;

    TranspositionTable<Game> transpositionTable;

    int timeLimitInMs{};
    std::chrono::high_resolution_clock::time_point startTime;

    bool debug{};

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

        auto savedAlpha = alpha, savedBeta = beta;
        auto entry = transpositionTable.find(state);
        if (entry.first && entry.second.depth <= depth)
        {
            auto value = entry.second.value;
            auto flag = entry.second.flag;
            if (flag == Flag::exact)
                return value;
            else if (flag == Flag::lowerBound)
                alpha = std::max(alpha, value);
            else if (flag == Flag::upperBound)
                beta = std::min(beta, value);

            if (alpha >= beta)
                return value;
        }

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

        if (bestValue <= savedAlpha)
            transpositionTable.set(state, bestValue, depth, Flag::upperBound);
        else if (bestValue >= savedBeta)
            transpositionTable.set(state, bestValue, depth, Flag::lowerBound);
        else
            transpositionTable.set(state, bestValue, depth, Flag::exact);

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
        std::stable_sort(
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
