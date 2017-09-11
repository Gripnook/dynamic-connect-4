#pragma once

#include <vector>
#include <array>
#include <set>
#include <cstdint>
#include <utility>
#include <tuple>
#include <algorithm>
#include <limits>
#include <functional>

class DynamicConnect4
{
public:
    static const size_t boardSize = 7;
    static const size_t piecesPerPlayer = 6;

    struct StateType
    {
        int32_t player{1};
        std::array<std::pair<size_t, size_t>, piecesPerPlayer> whitePieces{
            {std::make_pair(0, 2),
             std::make_pair(0, 4),
             std::make_pair(0, 6),
             std::make_pair(6, 1),
             std::make_pair(6, 3),
             std::make_pair(6, 5)}};
        std::array<std::pair<size_t, size_t>, piecesPerPlayer> blackPieces{
            {std::make_pair(0, 1),
             std::make_pair(0, 3),
             std::make_pair(0, 5),
             std::make_pair(6, 0),
             std::make_pair(6, 2),
             std::make_pair(6, 4)}};
    };

    enum class Direction
    {
        east,
        north,
        west,
        south
    };

    using ActionType = std::tuple<size_t, size_t, Direction>;
    using EvalType = double;

    std::vector<ActionType> getActions(const StateType& state) const
    {
        std::vector<ActionType> result;
        auto& pieces = state.player == 1 ? state.whitePieces : state.blackPieces;
        for (const auto& piece : pieces)
        {
            auto x = piece.first;
            auto y = piece.second;
            if (get(state, x + 1, y) == 0)
                result.emplace_back(x, y, Direction::east);
            if (get(state, x - 1, y) == 0)
                result.emplace_back(x, y, Direction::west);
            if (get(state, x, y + 1) == 0)
                result.emplace_back(x, y, Direction::south);
            if (get(state, x, y - 1) == 0)
                result.emplace_back(x, y, Direction::north);
        }
        return result;
    }

    StateType getResult(StateType state, const ActionType& action) const
    {
        auto x = std::get<0>(action);
        auto y = std::get<1>(action);
        auto direction = std::get<2>(action);

        auto& pieces = state.player == 1 ? state.whitePieces : state.blackPieces;
        auto it =
            std::find(std::begin(pieces), std::end(pieces), std::make_pair(x, y));

        switch (direction)
        {
        case Direction::east:
            *it = std::make_pair(x + 1, y);
            break;
        case Direction::west:
            *it = std::make_pair(x - 1, y);
            break;
        case Direction::south:
            *it = std::make_pair(x, y + 1);
            break;
        case Direction::north:
            *it = std::make_pair(x, y - 1);
            break;
        default:
            throw std::logic_error("impossible");
        }

        state.player = other(state.player);
        return state;
    }

    bool isTerminal(const StateType& state) const
    {
        return isWinner(state);
    }

    EvalType getUtility(const StateType& state) const
    {
        // Assume state is terminal.
        return state.player == 1 ? std::numeric_limits<EvalType>::lowest() :
                                   std::numeric_limits<EvalType>::max();
    }

private:
    bool isWinner(const StateType& state) const
    {
        // If it is the current player's turn, then the other player is the one
        // who may have won.
        auto player = other(state.player);

        return checkRows(player, state) || checkColumns(player, state) ||
            checkDiagonals(player, state) || checkAntiDiagonals(player, state);
    }

    bool checkRows(int32_t player, const StateType& state) const
    {
        auto pieces = player == 1 ? state.whitePieces : state.blackPieces;
        std::sort(std::begin(pieces), std::end(pieces));
        size_t row = -1, col = -1;
        size_t count = 0;
        for (const auto& piece : pieces)
        {
            if (piece.first == row && piece.second == col + 1)
                ++count;
            else
                count = 0;
            if (count >= 3)
                return true;
            row = piece.first;
            col = piece.second;
        }
        return false;
    }

    bool checkColumns(int32_t player, const StateType& state) const
    {
        auto pieces = player == 1 ? state.whitePieces : state.blackPieces;
        std::sort(
            std::begin(pieces),
            std::end(pieces),
            [](const std::pair<size_t, size_t>& lhs,
               const std::pair<size_t, size_t>& rhs) {
                return lhs.second == rhs.second ? lhs.first < rhs.first :
                                                  lhs.second < rhs.second;
            });
        size_t row = -1, col = -1;
        size_t count = 0;
        for (const auto& piece : pieces)
        {
            if (piece.first == row + 1 && piece.second == col)
                ++count;
            else
                count = 0;
            if (count >= 3)
                return true;
            row = piece.first;
            col = piece.second;
        }
        return false;
    }

    bool checkDiagonals(int32_t player, const StateType& state) const
    {
        auto pieces = player == 1 ? state.whitePieces : state.blackPieces;
        std::sort(
            std::begin(pieces),
            std::end(pieces),
            [&](const std::pair<size_t, size_t>& lhs,
                const std::pair<size_t, size_t>& rhs) {
                auto lhsDiag = boardSize + lhs.first - lhs.second;
                auto rhsDiag = boardSize + rhs.first - rhs.second;
                return lhsDiag == rhsDiag ? lhs.first < rhs.first :
                                            lhsDiag < rhsDiag;
            });
        size_t diag = -1, row = -1;
        size_t count = 0;
        for (const auto& piece : pieces)
        {
            if (boardSize + piece.first - piece.second == diag &&
                piece.first == row + 1)
                ++count;
            else
                count = 0;
            if (count >= 3)
                return true;
            diag = boardSize + piece.first - piece.second;
            row = piece.second;
        }
        return false;
    }

    bool checkAntiDiagonals(int32_t player, const StateType& state) const
    {
        auto pieces = player == 1 ? state.whitePieces : state.blackPieces;
        std::sort(
            std::begin(pieces),
            std::end(pieces),
            [](const std::pair<size_t, size_t>& lhs,
               const std::pair<size_t, size_t>& rhs) {
                auto lhsAntiDiag = lhs.first + lhs.second;
                auto rhsAntiDiag = rhs.first + rhs.second;
                return lhsAntiDiag == rhsAntiDiag ? lhs.first < rhs.first :
                                                    lhsAntiDiag < rhsAntiDiag;
            });
        size_t antiDiag = -1, row = -1;
        size_t count = 0;
        for (const auto& piece : pieces)
        {
            if (piece.first + piece.second == antiDiag && piece.first == row + 1)
                ++count;
            else
                count = 0;
            if (count >= 3)
                return true;
            antiDiag = piece.first + piece.second;
            row = piece.second;
        }
        return false;
    }

    int32_t other(int32_t player) const
    {
        return player == 1 ? 2 : 1;
    }

    int32_t get(const StateType& state, size_t x, size_t y) const
    {
        if (x >= boardSize || y >= boardSize)
            return -1;
        for (const auto& piece : state.whitePieces)
        {
            if (piece.first == x && piece.second == y)
                return 1;
        }
        for (const auto& piece : state.blackPieces)
        {
            if (piece.first == x && piece.second == y)
                return 2;
        }
        return 0;
    }
};
