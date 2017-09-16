#pragma once

#include <array>

#include "game.h"
#include "drawboard.h"

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
class ConnectedPiecesV1
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
class ConnectedPiecesV2
{
public:
    EvalType operator()(const StateType& state) const
    {
        Drawboard board{state};
        return eval(1, state, board) - eval(2, state, board);
    }

private:
    static const EvalType diagonalFactor;

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

const EvalType ConnectedPiecesV2::diagonalFactor = 1.21875f;

// A measure of how connected the pieces of each player are.
// This heuristic awards N*(N-1)/2 for each connected set of
// N pieces in a row, column, diagonal, or antidiagonal.
// In addition, it awards points for almost connected pieces,
// which are pieces separated by a single empty space.
class ConnectedPiecesV3
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
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (auto it1 = std::begin(pieces); it1 != std::end(pieces); ++it1)
        {
            for (auto it2 = it1 + 1; it2 != std::end(pieces); ++it2)
            {
                auto dx = std::abs(it2->x() - it1->x());
                auto dy = std::abs(it2->y() - it1->y());
                auto maxDistance = std::max(dx, dy);
                if (maxDistance == 1)
                {
                    result += 1.0f;
                }
                else if (maxDistance == 2 && (dx % 2 == 0) && (dy % 2 == 0))
                {
                    auto mx = (it2->x() + it1->x()) / 2;
                    auto my = (it2->y() + it1->y()) / 2;
                    auto value = board.get(mx, my);
                    if (value == player)
                        result += 1.0f;
                    else if (value == 0)
                    {
                        if (dx == 0 || dy == 0)
                            result += 0.40625f;
                        else
                            result += 0.09375f;
                    }
                }
            }
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
class CentralDominanceV1
{
public:
    EvalType operator()(const StateType& state) const
    {
        return eval(1, state) - eval(2, state);
    }

private:
    static const std::array<std::array<EvalType, boardSize>, boardSize> lookupTable;

    EvalType eval(int player, const StateType& state) const
    {
        EvalType result = 0;
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (const auto& piece : pieces)
            result += lookupTable[piece.x()][piece.y()];
        return result;
    }
};

const std::array<std::array<EvalType, boardSize>, boardSize>
    CentralDominanceV1::lookupTable{{
        {0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f},
        {0.0000f, 0.8125f, 1.0000f, 1.1875f, 1.0000f, 0.8125f, 0.0000f},
        {0.0000f, 1.0000f, 2.0000f, 2.1875f, 2.0000f, 1.0000f, 0.0000f},
        {0.0000f, 1.1875f, 2.1875f, 2.3750f, 2.1875f, 1.1875f, 0.0000f},
        {0.0000f, 1.0000f, 2.0000f, 2.1875f, 2.0000f, 1.0000f, 0.0000f},
        {0.0000f, 0.8125f, 1.0000f, 1.1875f, 1.0000f, 0.8125f, 0.0000f},
        {0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f},
    }};

// A measure of a player's domination of the center of the board.
// This heuristic awards a score to each piece based on a lookup table,
// with central positions being worth more points.
// This version awards bonuses when a player has at least 3 pieces
// in the center of the board.
class CentralDominanceV2
{
public:
    EvalType operator()(const StateType& state) const
    {
        return eval(1, state) - eval(2, state);
    }

private:
    static const std::array<std::array<EvalType, boardSize>, boardSize> lookupTable;

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
        // Give a bonus to a high density of central pieces.
        if (count == 3)
            result *= 1.09375f;
        if (count == 4)
            result *= 1.25f;
        if (count > 4)
            result *= 1.21875f;
        return result;
    }
};

const std::array<std::array<EvalType, boardSize>, boardSize>
    CentralDominanceV2::lookupTable{{
        {0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f},
        {0.0000f, 0.8125f, 1.0000f, 1.1875f, 1.0000f, 0.8125f, 0.0000f},
        {0.0000f, 1.0000f, 2.0000f, 2.1875f, 2.0000f, 1.0000f, 0.0000f},
        {0.0000f, 1.1875f, 2.1875f, 2.3750f, 2.1875f, 1.1875f, 0.0000f},
        {0.0000f, 1.0000f, 2.0000f, 2.1875f, 2.0000f, 1.0000f, 0.0000f},
        {0.0000f, 0.8125f, 1.0000f, 1.1875f, 1.0000f, 0.8125f, 0.0000f},
        {0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f},
    }};

// A measure of how blocked a player's pieces are in the early game.
// This heuristic decreases the score of any piece that is blocked when
// considering moves that go towards the center of the board.
// The player that is using the heuristic has a higher weight associated with
// these penalties, ensuring that they will try to unblock themselves before
// trying to block the opponent.
template <int Player>
class EarlyBlocking
{
public:
    EvalType operator()(const StateType& state) const
    {
        Drawboard board{state};
        if (Player == 1)
            return eval(1, state, board) - 0.5f * eval(2, state, board);
        else
            return 0.5f * eval(1, state, board) - eval(2, state, board);
    }

private:
    static const EvalType forwardBlockingFactor;
    static const EvalType strongDiagonalBlockingFactor;
    static const EvalType weakDiagonalBlockingFactor;

    EvalType
        eval(int player, const StateType& state, const Drawboard& board) const
    {
        EvalType result = 0;
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        int otherPlayer = player == 1 ? 2 : 1;
        for (const auto& piece : pieces)
        {
            auto x = piece.x();
            auto y = piece.y();

            if (2 * x + 1 == boardSize)
                continue;
            auto xForward = (2 * x + 1 < boardSize) ? x + 1 : x - 1;

            auto upperDiagonalFactor = (2 * y + 1 > boardSize) ?
                strongDiagonalBlockingFactor :
                weakDiagonalBlockingFactor;
            auto lowerDiagonalFactor = (2 * y + 1 < boardSize) ?
                strongDiagonalBlockingFactor :
                weakDiagonalBlockingFactor;

            if (board.get(xForward, y) == otherPlayer)
                result -= forwardBlockingFactor;
            if (board.get(xForward, y - 1) == otherPlayer)
                result -= upperDiagonalFactor;
            if (board.get(xForward, y + 1) == otherPlayer)
                result -= lowerDiagonalFactor;
        }
        return result;
    }
};

template <int Player>
const EvalType EarlyBlocking<Player>::forwardBlockingFactor = 1.0f;

template <int Player>
const EvalType EarlyBlocking<Player>::strongDiagonalBlockingFactor = 0.8125f;

template <int Player>
const EvalType EarlyBlocking<Player>::weakDiagonalBlockingFactor = 0.59375f;
}
