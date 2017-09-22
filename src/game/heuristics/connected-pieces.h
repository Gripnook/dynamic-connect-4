#pragma once

#include "game/game.h"
#include "game/drawboard.h"

namespace DynamicConnect4 {

using StateType = Game::StateType;
using ActionType = Game::ActionType;
using EvalType = Game::EvalType;

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

// A measure of how connected the pieces of each player are.
// This heuristic awards (N-1)^2 for each connected set of
// N pieces in a row, column, diagonal, or antidiagonal.
class ConnectedPiecesV4
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
            int count = 0;
            for (int i = 1; board.get(x + i, y - i) == player; ++i)
                ++count;
            if (count > 0)
                result += 2 * count - 1;

            count = 0;
            for (int i = 1; board.get(x + i, y) == player; ++i)
                ++count;
            if (count > 0)
                result += 2 * count - 1;

            count = 0;
            for (int i = 1; board.get(x + i, y + i) == player; ++i)
                ++count;
            if (count > 0)
                result += 2 * count - 1;

            count = 0;
            for (int i = 1; board.get(x, y + i) == player; ++i)
                ++count;
            if (count > 0)
                result += 2 * count - 1;
        }
        return result;
    }
};
}
