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
        std::function<bool(EvalType, EvalType)> comp;
        if (isMax)
            comp = std::greater<EvalType>{};
        else
            comp = std::less<EvalType>{};

        auto alpha = std::numeric_limits<EvalType>::lowest();
        auto beta = std::numeric_limits<EvalType>::max();

        auto actions = game.getActions(state);
        auto bestAction = std::make_pair(actions.front(), init);
        for (const auto& action : actions)
        {
            auto value =
                alphaBeta(game.getResult(state, action), alpha, beta, 1, !isMax);
            if (comp(value, alpha))
                alpha = value;
            if (comp(value, bestAction.second))
                bestAction = std::make_pair(action, value);
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
                if (bestValue >= beta)
                    break;
                alpha = std::max(alpha, bestValue);
            }
            else
            {
                bestValue = std::min(bestValue, value);
                if (bestValue <= alpha)
                    break;
                beta = std::min(beta, bestValue);
            }
        }
        return bestValue;
    }

    Game& game;
    int maxDepth;
    int count{0};
    Heuristic heuristic;
};
}
