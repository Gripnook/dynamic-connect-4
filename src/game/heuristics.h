#pragma once

#include "game/game.h"
#include "game/heuristics/connected-pieces.h"
#include "game/heuristics/central-dominance.h"

namespace DynamicConnect4 {

using StateType = Game::StateType;
using EvalType = Game::EvalType;

// A template for combining different heuristics with weights.
template <typename T, typename... Args>
class Heuristic
{
public:
    template <typename... EvalTypes>
    Heuristic(EvalType weight, EvalTypes... others)
        : weight{weight}, others{others...}
    {
    }

    EvalType operator()(const StateType& state) const
    {
        return weight * first(state) + others(state);
    }

private:
    T first;
    EvalType weight;
    Heuristic<Args...> others;
};

// The base specialization for one heuristic.
template <typename T>
class Heuristic<T>
{
public:
    Heuristic(EvalType weight = 1) : weight{weight}
    {
    }

    EvalType operator()(const StateType& state) const
    {
        return weight * heuristic(state);
    }

private:
    T heuristic;
    EvalType weight;
};
}
