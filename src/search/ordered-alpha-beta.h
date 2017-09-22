#pragma once

#include <map>
#include <vector>
#include <limits>
#include <algorithm>
#include <utility>
#include <functional>

namespace Search {

// A class implementing an ordered version of the alpha-beta search algorithm
// for a game of type Game. This version orders child nodes using the heuristic
// function before searching them.
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
class OrderedAlphaBeta
{
public:
    using StateType = typename Game::StateType;
    using ActionType = typename Game::ActionType;
    using EvalType = typename Game::EvalType;

    using Heuristic = std::function<EvalType(const StateType&)>;

    OrderedAlphaBeta(Game& game) : game(game)
    {
    }

    ActionType
        search(const StateType& state, Heuristic heuristic, int depth, bool isMax)
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
        // Sorting the actions using the heuristic helps us consider
        // the best actions first.
        if (depth > 1)
            actions = heuristicSort(actions, state, comp);
        auto bestAction = std::make_pair(actions.front(), init);
        for (const auto& action : actions)
        {
            auto value = alphaBeta(
                game.getResult(state, action), alpha, beta, depth - 1, !isMax);
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
        this->depth = depth;
        return bestAction.first;
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
    int depth{};
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
        else if (depth == 0)
            return heuristic(state);

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
        if (depth > 1)
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
};
}
