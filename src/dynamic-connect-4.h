#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <utility>
#include <tuple>
#include <algorithm>
#include <limits>

class DynamicConnect4
{
public:
    static const size_t boardSize = 7;
    static const size_t piecesPerPlayer = 6;

    struct StateType
    {
        int32_t player{1};
        std::array<std::array<int32_t, boardSize>, boardSize> board{};
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

        StateType()
        {
            for (const auto& piece : whitePieces)
                board[piece.first][piece.second] = 1;
            for (const auto& piece : blackPieces)
                board[piece.first][piece.second] = 2;
        }
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

    const size_t depth;

    DynamicConnect4(size_t depth) : depth{depth}
    {
    }

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

        state.board[x][y] = 0;
        switch (direction)
        {
        case Direction::east:
            state.board[x + 1][y] = state.player;
            *it = std::make_pair(x + 1, y);
            break;
        case Direction::west:
            state.board[x - 1][y] = state.player;
            *it = std::make_pair(x - 1, y);
            break;
        case Direction::south:
            state.board[x][y + 1] = state.player;
            *it = std::make_pair(x, y + 1);
            break;
        case Direction::north:
            state.board[x][y - 1] = state.player;
            *it = std::make_pair(x, y - 1);
            break;
        default:
            throw std::logic_error("impossible");
        }

        state.player = other(state.player);
        return state;
    }

    bool cutoffTest(const StateType& state, size_t depth) const
    {
        if (depth > this->depth)
            return true;
        return isTerminal(state);
    }

    EvalType eval(const StateType& state) const
    {
        // TODO
        if (isTerminal(state))
            return getUtility(state);

        return evalRows(1, state) + evalColumns(1, state) - evalRows(2, state) -
            evalColumns(2, state);
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

        return checkRows(player, state) || checkColumns(player, state);
    }

    bool checkRows(int32_t player, const StateType& state) const
    {
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (size_t row = 0; row < boardSize; ++row)
        {
            std::vector<size_t> cols;
            for (const auto& piece : pieces)
                if (piece.first == row)
                    cols.push_back(piece.second);
            if (cols.size() < 4)
                continue;
            std::sort(std::begin(cols), std::end(cols));
            std::adjacent_difference(
                std::begin(cols), std::end(cols), std::begin(cols));
            size_t count = 0;
            std::for_each(
                std::begin(cols) + 1, std::end(cols), [&](const auto& col) {
                    if (count >= 3)
                        return;
                    if (col == 1)
                        ++count;
                    else
                        count = 0;
                });
            if (count >= 3)
                return true;
        }
        return false;
    }

    bool checkColumns(int32_t player, const StateType& state) const
    {
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (size_t col = 0; col < boardSize; ++col)
        {
            std::vector<size_t> rows;
            for (const auto& piece : pieces)
                if (piece.second == col)
                    rows.push_back(piece.second);
            if (rows.size() < 4)
                continue;
            std::sort(std::begin(rows), std::end(rows));
            std::adjacent_difference(
                std::begin(rows), std::end(rows), std::begin(rows));
            size_t count = 0;
            std::for_each(
                std::begin(rows) + 1, std::end(rows), [&](const auto& row) {
                    if (count >= 3)
                        return;
                    if (row == 1)
                        ++count;
                    else
                        count = 0;
                });
            if (count >= 3)
                return true;
        }
        return false;
    }

    EvalType evalRows(int32_t player, const StateType& state) const
    {
        EvalType result = 0;
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (size_t row = 0; row < boardSize; ++row)
        {
            std::vector<size_t> cols;
            for (const auto& piece : pieces)
                if (piece.first == row)
                    cols.push_back(piece.second);
            if (cols.empty())
                continue;
            std::sort(std::begin(cols), std::end(cols));
            std::adjacent_difference(
                std::begin(cols), std::end(cols), std::begin(cols));
            size_t count = 0;
            std::for_each(
                std::begin(cols) + 1, std::end(cols), [&](const auto& col) {
                    if (col == 1)
                        ++count;
                    else
                        count = 0;
                    result += count;
                });
        }
        return result;
    }

    EvalType evalColumns(int32_t player, const StateType& state) const
    {
        EvalType result = 0;
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (size_t col = 0; col < boardSize; ++col)
        {
            std::vector<size_t> rows;
            for (const auto& piece : pieces)
                if (piece.second == col)
                    rows.push_back(piece.second);
            if (rows.empty())
                continue;
            std::sort(std::begin(rows), std::end(rows));
            std::adjacent_difference(
                std::begin(rows), std::end(rows), std::begin(rows));
            size_t count = 0;
            std::for_each(
                std::begin(rows) + 1, std::end(rows), [&](const auto& row) {
                    if (row == 1)
                        ++count;
                    else
                        count = 0;
                    result += count;
                });
        }
        return result;
    }

    int32_t other(int32_t player) const
    {
        return player == 1 ? 2 : 1;
    }

    int32_t get(const StateType& state, size_t x, size_t y) const
    {
        if (x >= boardSize || y >= boardSize)
            return -1;
        return state.board[x][y];
    }
};
