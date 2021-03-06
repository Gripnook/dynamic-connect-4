#pragma once

#include <map>
#include <vector>
#include <limits>
#include <algorithm>
#include <utility>
#include <functional>
#include <chrono>
#include <iostream>
#include <atomic>

#include "search/transposition-table.h"

namespace Search {

// A class implementing an iterative version of the alpha-beta search algorithm
// for a game of type Game. This algorithm is optimized in several ways:
//      1) It performs an iterative depth search until it runs out of time. It
//          keeps the possible moves for the root in an array, which it sorts
//          after each iteration using a stable sort. This allows it to get
//          better move ordering in the next iteration. Being a stable sort, it
//          also ensures that actions keep their relative ordering from previous
//          iterations should they compare equally in this one.
//      2) The moves at every node of the search tree other than the root are
//          sorted according to their heuristic values to allow for better move
//          ordering. The exception is those nodes at a depth d with branching
//          factor b for which the cost of sorting (c*b*lg(b)) and evaluating
//          heuristics (h*b) is mathematically higher than the benefit obtained
//          through ordering (h*b^floor(d/2)). This corresponds to sorting for
//          nodes of depth at least 4. Note that in this case, the depth is
//          measured as the distance from the leaves of the search tree.
//      3) A transposition table is used to keep track of the moves seen so far.
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
class IterativeAlphaBeta
{
public:
    using StateType = typename Game::StateType;
    using ActionType = typename Game::ActionType;
    using EvalType = typename Game::EvalType;

    using Heuristic = std::function<EvalType(const StateType&)>;

    IterativeAlphaBeta(Game& game, bool debug = false)
        : game(game), transpositionTable{4 * 1024 * 1024}, debug{debug}
    {
    }

    ActionType search(
        const StateType& state,
        Heuristic heuristic,
        int timeLimitInMs,
        bool isMax)
    {
        count = 1;
        this->heuristic = heuristic;
        this->timeLimitInMs = timeLimitInMs;
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
            auto alpha = std::numeric_limits<EvalType>::lowest();
            auto beta = std::numeric_limits<EvalType>::max();

            for (const auto& action : actions)
            {
                auto value = alphaBeta(
                    game.getResult(state, action),
                    alpha,
                    beta,
                    depth - 1,
                    !isMax);
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
                          << depth << " with " << transpositionTable.size()
                          << " nodes cached and a hit rate of "
                          << transpositionTable.getHitRate() << std::endl;
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

            if (values[actions.front()] == lossIndicator)
            {
                // If we are guaranteed to lose, it is better to return now and
                // clear the transposition table. This is because we want to
                // recompute the path to the most distant loss in the next move
                // in case we are playing against a non-optimal player or an
                // optimal player with restricted search depth.
                transpositionTable.clear();
                this->depth = depth;
                return actions.front();
            }
        }
    }

    void stop()
    {
        timeLimitInMs = 0;
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
    int count{0};
    int depth{0};
    Heuristic heuristic;

    TranspositionTable<Game> transpositionTable;

    std::atomic<int> timeLimitInMs{};
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
        else if (depth == 0 || isTimeUp())
            return heuristic(state);

        auto savedAlpha = alpha, savedBeta = beta;
        auto entry = transpositionTable.find(state);
        if (entry.first && entry.second.depth >= depth)
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
        if (depth >= 4)
            actions = heuristicSort(actions, state, comp);
        for (const auto& action : actions)
        {
            auto value = alphaBeta(
                game.getResult(state, action), alpha, beta, depth - 1, !isMax);
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

        // We only save the result if we didn't run out of time,
        // since it means we were able to search the full depth.
        if (!isTimeUp())
        {
            if (bestValue <= savedAlpha)
                transpositionTable.emplace(
                    state, bestValue, depth, Flag::upperBound);
            else if (bestValue >= savedBeta)
                transpositionTable.emplace(
                    state, bestValue, depth, Flag::lowerBound);
            else
                transpositionTable.emplace(
                    state, bestValue, depth, Flag::exact);
        }

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
        // It is very important that the sort is stable, since it ensures that
        // actions keep their relative ordering from previous sorts should they
        // now be equal.
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
