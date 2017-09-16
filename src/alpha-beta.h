#pragma once

#include <limits>
#include <algorithm>
#include <utility>
#include <functional>

namespace Search {

template <typename Game>
class AlphaBeta
{
public:
    using StateType = typename Game::StateType;
    using ActionType = typename Game::ActionType;
    using EvalType = typename Game::EvalType;

    using Heuristic = std::function<EvalType(const StateType&)>;

    AlphaBeta(Game& game, int maxDepth) : game(game), maxDepth{maxDepth}
    {
    }

    ActionType search(const StateType& state, Heuristic heuristic, bool isMax)
    {
        count = 1;
        this->heuristic = heuristic;

        auto init = isMax ? std::numeric_limits<EvalType>::lowest() :
                            std::numeric_limits<EvalType>::max();

        auto alpha = std::numeric_limits<EvalType>::lowest();
        auto beta = std::numeric_limits<EvalType>::max();

        auto actions = game.getActions(state);
        auto bestAction = std::make_pair(actions.front(), init);
        for (const auto& action : actions)
        {
            auto value =
                alphaBeta(game.getResult(state, action), alpha, beta, 1, !isMax);
            if (isMax)
            {
                alpha = std::max(alpha, value);
                if (value > bestAction.second)
                    bestAction = std::make_pair(action, value);
            }
            else
            {
                beta = std::min(beta, value);
                if (value < bestAction.second)
                    bestAction = std::make_pair(action, value);
            }
            if (alpha >= beta)
                break;
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
    Game& game;
    int maxDepth{};
    int count{0};
    Heuristic heuristic;

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
        else if (depth >= maxDepth)
            return heuristic(state);

        auto init = isMax ? std::numeric_limits<EvalType>::lowest() :
                            std::numeric_limits<EvalType>::max();

        auto bestValue = init;
        auto actions = game.getActions(state);
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
        return bestValue;
    }
};
}
