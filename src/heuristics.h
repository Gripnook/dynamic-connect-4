#pragma once

#include "dynamic-connect-4.h"

using StateType = typename DynamicConnect4::StateType;
using ActionType = typename DynamicConnect4::ActionType;
using EvalType = typename DynamicConnect4::EvalType;

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
    Heuristic(EvalType weight) : weight{weight}
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
        auto evalPlayer1 = evalRows(1, state) + evalColumns(1, state) +
            evalDiagonals(1, state) + evalAntiDiagonals(1, state);
        auto evalPlayer2 = evalRows(2, state) + evalColumns(2, state) +
            evalDiagonals(2, state) + evalAntiDiagonals(2, state);
        return evalPlayer1 - evalPlayer2;
    }

private:
    EvalType evalRows(int player, const StateType& state) const
    {
        EvalType result = 0;
        auto pieces = player == 1 ? state.whitePieces : state.blackPieces;
        std::sort(std::begin(pieces), std::end(pieces));
        int row = -1, col = -1;
        int count = 0;
        for (const auto& piece : pieces)
        {
            if (piece.first == row && piece.second == col + 1)
                ++count;
            else
                count = 0;
            result += count;
            row = piece.first;
            col = piece.second;
        }
        return result;
    }

    EvalType evalColumns(int player, const StateType& state) const
    {
        EvalType result = 0;
        auto pieces = player == 1 ? state.whitePieces : state.blackPieces;
        std::sort(
            std::begin(pieces),
            std::end(pieces),
            [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) {
                return lhs.second == rhs.second ? lhs.first < rhs.first :
                                                  lhs.second < rhs.second;
            });
        int row = -1, col = -1;
        int count = 0;
        for (const auto& piece : pieces)
        {
            if (piece.first == row + 1 && piece.second == col)
                ++count;
            else
                count = 0;
            result += count;
            row = piece.first;
            col = piece.second;
        }
        return result;
    }

    EvalType evalDiagonals(int player, const StateType& state) const
    {
        EvalType result = 0;
        auto pieces = player == 1 ? state.whitePieces : state.blackPieces;
        std::sort(
            std::begin(pieces),
            std::end(pieces),
            [&](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) {
                auto lhsDiag =
                    DynamicConnect4::boardSize + lhs.first - lhs.second;
                auto rhsDiag =
                    DynamicConnect4::boardSize + rhs.first - rhs.second;
                return lhsDiag == rhsDiag ? lhs.first < rhs.first :
                                            lhsDiag < rhsDiag;
            });
        int diag = -1, row = -1;
        int count = 0;
        for (const auto& piece : pieces)
        {
            if (DynamicConnect4::boardSize + piece.first - piece.second == diag &&
                piece.first == row + 1)
                ++count;
            else
                count = 0;
            result += count;
            diag = DynamicConnect4::boardSize + piece.first - piece.second;
            row = piece.first;
        }
        return result;
    }

    EvalType evalAntiDiagonals(int player, const StateType& state) const
    {
        EvalType result = 0;
        auto pieces = player == 1 ? state.whitePieces : state.blackPieces;
        std::sort(
            std::begin(pieces),
            std::end(pieces),
            [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) {
                auto lhsAntiDiag = lhs.first + lhs.second;
                auto rhsAntiDiag = rhs.first + rhs.second;
                return lhsAntiDiag == rhsAntiDiag ? lhs.first < rhs.first :
                                                    lhsAntiDiag < rhsAntiDiag;
            });
        int antiDiag = -1, row = -1;
        int count = 0;
        for (const auto& piece : pieces)
        {
            if (piece.first + piece.second == antiDiag && piece.first == row + 1)
                ++count;
            else
                count = 0;
            result += count;
            antiDiag = piece.first + piece.second;
            row = piece.first;
        }
        return result;
    }
};

class Proximity
{
public:
    EvalType operator()(const StateType& state)
    {
        return evalProximity(1, state) - evalProximity(2, state);
    }

private:
    EvalType evalProximity(int player, const StateType& state) const
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

class CentralDomination
{
public:
    EvalType operator()(const StateType& state)
    {
        return evalCenter(1, state) - evalCenter(2, state);
    }

private:
    EvalType evalCenter(int player, const StateType& state) const
    {
        EvalType result = 0;
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (const auto& piece : pieces)
        {
            result += std::min(
                piece.first, DynamicConnect4::boardSize - 1 - piece.first);
            result += std::min(
                piece.second, DynamicConnect4::boardSize - 1 - piece.second);
        }
        return result;
    }
};
