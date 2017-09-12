#pragma once

#include <array>

#include "dynamic-connect-4.h"

using StateType = DynamicConnect4::StateType;
using ActionType = DynamicConnect4::ActionType;
using EvalType = DynamicConnect4::EvalType;
using Drawboard = DynamicConnect4::Drawboard;

template <typename T, typename... Args>
class Heuristic
{
public:
    template <typename... EvalTypes>
    Heuristic(EvalType weight, EvalTypes... others)
        : weight{weight}, others{others...}
    {
    }

    EvalType operator()(const StateType& state)
    {
        return weight * first(state) + others(state);
    }

private:
    T first;
    EvalType weight;
    Heuristic<Args...> others;
};

template <typename T>
class Heuristic<T>
{
public:
    Heuristic(EvalType weight = 1) : weight{weight}
    {
    }

    EvalType operator()(const StateType& state)
    {
        return weight * heuristic(state);
    }

private:
    T heuristic;
    EvalType weight;
};

class ConsecutiveElements
{
public:
    EvalType operator()(const StateType& state)
    {
        Drawboard board{state};
        return eval(1, state, board) - eval(2, state, board);
    }

private:
    EvalType
        eval(int player, const StateType& state, const Drawboard& board) const
    {
        EvalType result = 0;
        auto pieces = player == 1 ? state.whitePieces : state.blackPieces;
        std::sort(std::begin(pieces), std::end(pieces));
        for (const auto& piece : pieces)
        {
            auto x = piece.first;
            auto y = piece.second;
            for (int i = 1; board.get(x + i, y - i) == player; ++i)
                ++result;
            for (int i = 1; board.get(x + i, y) == player; ++i)
                ++result;
            for (int i = 1; board.get(x + i, y + i) == player; ++i)
                ++result;
            for (int i = 1; board.get(x, y + i) == player; ++i)
                ++result;
        }
        return result;
    }
};

class KillerConsecutiveElements
{
public:
    EvalType operator()(const StateType& state)
    {
        Drawboard board{state};
        return eval(1, state, board) - eval(2, state, board);
    }

private:
    EvalType
        eval(int player, const StateType& state, const Drawboard& board) const
    {
        EvalType result = 0;
        auto pieces = player == 1 ? state.whitePieces : state.blackPieces;
        std::sort(std::begin(pieces), std::end(pieces));
        for (const auto& piece : pieces)
        {
            auto x = piece.first;
            auto y = piece.second;
            int i = 0;
            for (i = 1; board.get(x + i, y - i) == player; ++i)
                ++result;
            if (i == 3)
            {
                bool emptyFront = board.get(x - 1, y + 1) == 0;
                bool emptyBack = board.get(x + i, y - i) == 0;
                if (emptyFront && emptyBack)
                    result += 5;
                else if (emptyFront || emptyBack)
                    result += 2;
            }
            for (i = 1; board.get(x + i, y) == player; ++i)
                ++result;
            if (i == 3)
            {
                bool emptyFront = board.get(x - 1, y) == 0;
                bool emptyBack = board.get(x + i, y) == 0;
                if (emptyFront && emptyBack)
                    result += 5;
                else if (emptyFront || emptyBack)
                    result += 2;
            }
            for (i = 1; board.get(x + i, y + i) == player; ++i)
                ++result;
            if (i == 3)
            {
                bool emptyFront = board.get(x - 1, y - 1) == 0;
                bool emptyBack = board.get(x + i, y + i) == 0;
                if (emptyFront && emptyBack)
                    result += 5;
                else if (emptyFront || emptyBack)
                    result += 2;
            }
            for (i = 1; board.get(x, y + i) == player; ++i)
                ++result;
            if (i == 3)
            {
                bool emptyFront = board.get(x, y - 1) == 0;
                bool emptyBack = board.get(x, y + i) == 0;
                if (emptyFront && emptyBack)
                    result += 5;
                else if (emptyFront || emptyBack)
                    result += 2;
            }
        }
        return result;
    }
};

class Proximity
{
public:
    EvalType operator()(const StateType& state)
    {
        return eval(1, state) - eval(2, state);
    }

private:
    EvalType eval(int player, const StateType& state) const
    {
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        auto maxRow =
            std::max_element(std::begin(pieces), std::end(pieces))->first;
        auto minRow =
            std::min_element(std::begin(pieces), std::end(pieces))->first;
        auto maxCol = std::max_element(
                          std::begin(pieces),
                          std::end(pieces),
                          [](const std::pair<int, int>& lhs,
                             const std::pair<int, int>& rhs) {
                              return lhs.second < rhs.second;
                          })
                          ->second;
        auto minCol = std::min_element(
                          std::begin(pieces),
                          std::end(pieces),
                          [](const std::pair<int, int>& lhs,
                             const std::pair<int, int>& rhs) {
                              return lhs.second < rhs.second;
                          })
                          ->second;

        return (DynamicConnect4::boardSize * DynamicConnect4::boardSize) -
            ((maxRow - minRow + 1) * (maxCol - minCol + 1));
    }
};

class CentralDominance
{
public:
    EvalType operator()(const StateType& state)
    {
        return eval(1, state) - eval(2, state);
    }

private:
    static const std::array<
        std::array<int8_t, DynamicConnect4::boardSize>,
        DynamicConnect4::boardSize>
        lookupTable;

    EvalType eval(int player, const StateType& state) const
    {
        EvalType result = 0;
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (const auto& piece : pieces)
            result += lookupTable[piece.first][piece.second];
        return result;
    }
};

const std::array<std::array<int8_t, DynamicConnect4::boardSize>, DynamicConnect4::boardSize>
    CentralDominance::lookupTable{{
        {0, 1, 1, 1, 1, 1, 0},
        {1, 2, 2, 3, 2, 2, 1},
        {1, 2, 3, 4, 3, 2, 1},
        {1, 3, 4, 5, 4, 3, 1},
        {1, 2, 3, 4, 3, 2, 1},
        {1, 2, 2, 3, 2, 2, 1},
        {0, 1, 1, 1, 1, 1, 0},
    }};
