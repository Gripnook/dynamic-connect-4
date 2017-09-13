#pragma once

#include <array>

#include "game/game.h"

namespace DynamicConnect4 {

using StateType = Game::StateType;
using ActionType = Game::ActionType;
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

// A measure of how connected the pieces of each player are.
// This heuristic awards N*(N-1)/2 for each connected set of
// N pieces in a row, column, diagonal, or antidiagonal.
class ConsecutiveElements
{
public:
    EvalType operator()(const StateType& state) const
    {
        Drawboard board{state};
        return eval(1, state, board) - eval(2, state, board);
    }

private:
    EvalType
        eval(int player, const StateType& state, const Drawboard& board) const
    {
        EvalType result = 0;
        // Since the pieces are sorted, we only need to look forward in the row,
        // column, diagonal, and antidiagonal for each piece.
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (const auto& piece : pieces)
        {
            auto x = piece.x();
            auto y = piece.y();
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

// A measure of how connected the pieces of each player are.
// This heuristic awards D*N*(N-1)/2 for each connected set of
// N pieces in a row, column, diagonal, or antidiagonal,
// where D is a scaling factor for diagonals, which are more important.
class ConnectedElements
{
public:
    EvalType operator()(const StateType& state) const
    {
        Drawboard board{state};
        return eval(1, state, board) - eval(2, state, board);
    }

private:
    EvalType
        eval(int player, const StateType& state, const Drawboard& board) const
    {
        static const EvalType diagonalFactor = 1.2f;

        EvalType result = 0;
        // Since the pieces are sorted, we only need to look forward in the row,
        // column, diagonal, and antidiagonal for each piece.
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (const auto& piece : pieces)
        {
            auto x = piece.x();
            auto y = piece.y();
            for (int i = 1; board.get(x + i, y - i) == player; ++i)
                result += diagonalFactor;
            for (int i = 1; board.get(x + i, y) == player; ++i)
                ++result;
            for (int i = 1; board.get(x + i, y + i) == player; ++i)
                result += diagonalFactor;
            for (int i = 1; board.get(x, y + i) == player; ++i)
                ++result;
        }
        return result;
    }
};

// A measure of how close the pieces of each player are to each other.
// This heuristic awards a score of BOARD_AREA - AREA, where AREA is
// the area of the smallest rectangle enclosing all of the player's pieces.
class Proximity
{
public:
    EvalType operator()(const StateType& state) const
    {
        return eval(1, state) - eval(2, state);
    }

private:
    EvalType eval(int player, const StateType& state) const
    {
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        int maxRow = 0, minRow = boardSize - 1;
        int maxCol = 0, minCol = boardSize - 1;
        for (const auto& piece : pieces)
        {
            maxRow = std::max(maxRow, piece.x());
            minRow = std::min(minRow, piece.x());
            maxCol = std::max(maxCol, piece.y());
            minCol = std::min(minCol, piece.y());
        }
        int area = (maxRow - minRow + 1) * (maxCol - minCol + 1);
        int boardArea = boardSize * boardSize;
        return boardArea - area;
    }
};

// A measure of a player's domination of the center of the board.
// This heuristic awards a score to each piece based on a lookup table,
// with central positions being worth more points.
class CentralDominance
{
public:
    EvalType operator()(const StateType& state) const
    {
        return eval(1, state) - eval(2, state);
    }

private:
    static const std::array<std::array<int8_t, boardSize>, boardSize> lookupTable;

    EvalType eval(int player, const StateType& state) const
    {
        EvalType result = 0;
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (const auto& piece : pieces)
            result += lookupTable[piece.x()][piece.y()];
        return result;
    }
};

const std::array<std::array<int8_t, boardSize>, boardSize>
    CentralDominance::lookupTable{{
        {0, 1, 1, 1, 1, 1, 0},
        {1, 3, 3, 3, 3, 3, 1},
        {1, 3, 4, 4, 4, 3, 1},
        {1, 3, 4, 4, 4, 3, 1},
        {1, 3, 4, 4, 4, 3, 1},
        {1, 3, 3, 3, 3, 3, 1},
        {0, 1, 1, 1, 1, 1, 0},
    }};

// A measure of a player's domination of the center of the board.
// This heuristic awards a score to each piece based on a lookup table,
// with central positions being worth more points.
// This version is designed for early game play, where getting your pieces to
// the center of the board matters a lot more.
class EarlyCentralDominance
{
public:
    EvalType operator()(const StateType& state) const
    {
        return eval(1, state) - eval(2, state);
    }

private:
    static const std::array<std::array<int8_t, boardSize>, boardSize> lookupTable;

    EvalType eval(int player, const StateType& state) const
    {
        EvalType result = 0;
        int count = 0;
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (const auto& piece : pieces)
        {
            auto value = lookupTable[piece.x()][piece.y()];
            result += value;
            if (value > 0)
                ++count;
        }
        if (count >= 4)
            result *= 2;
        return result;
    }
};

const std::array<std::array<int8_t, boardSize>, boardSize>
    EarlyCentralDominance::lookupTable{{
        {0, 0, 0, 0, 0, 0, 0},
        {0, 1, 1, 2, 1, 1, 0},
        {0, 1, 4, 5, 4, 1, 0},
        {0, 2, 5, 6, 5, 2, 0},
        {0, 1, 4, 5, 4, 1, 0},
        {0, 1, 1, 2, 1, 1, 0},
        {0, 0, 0, 0, 0, 0, 0},
    }};
}
