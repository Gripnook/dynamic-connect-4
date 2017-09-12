#pragma once

#include <vector>
#include <array>
#include <set>
#include <utility>
#include <tuple>
#include <algorithm>
#include <limits>
#include <functional>
#include <string>
#include <sstream>

class DynamicConnect4
{
public:
    static const int boardSize = 7;
    static const int piecesPerPlayer = 6;

    struct StateType
    {
        int player{1};
        std::array<std::pair<int, int>, piecesPerPlayer> whitePieces{
            {std::make_pair(0, 2),
             std::make_pair(0, 4),
             std::make_pair(0, 6),
             std::make_pair(6, 1),
             std::make_pair(6, 3),
             std::make_pair(6, 5)}};
        std::array<std::pair<int, int>, piecesPerPlayer> blackPieces{
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

    using ActionType = std::tuple<int, int, Direction>;
    using EvalType = double;

    // A small array to use as scratch paper when computing things.
    class Drawboard
    {
    public:
        Drawboard(StateType state) : state{state}
        {
            for (const auto& piece : state.whitePieces)
                drawboard[piece.first][piece.second] = 1;
            for (const auto& piece : state.blackPieces)
                drawboard[piece.first][piece.second] = 2;
        }

        ~Drawboard()
        {
            for (const auto& piece : state.whitePieces)
                drawboard[piece.first][piece.second] = 0;
            for (const auto& piece : state.blackPieces)
                drawboard[piece.first][piece.second] = 0;
            for (const auto& set : sets)
                drawboard[set.first][set.second] = 0;
        }

        int get(int x, int y) const
        {
            if (x < 0 || x >= boardSize || y < 0 || y >= boardSize)
                return -1;
            return drawboard[x][y];
        }

        void set(int x, int y, int8_t value)
        {
            if (x < 0 || x >= boardSize || y < 0 || y >= boardSize)
                return;
            if (drawboard[x][y] == 0 && value != 0)
                sets.emplace_back(x, y);
            drawboard[x][y] = value;
        }

    private:
        static std::array<std::array<int8_t, boardSize>, boardSize> drawboard;

        StateType state;
        std::vector<std::pair<int, int>> sets;
    };

    std::vector<ActionType> getActions(const StateType& state) const
    {
        Drawboard board{state};

        std::vector<ActionType> result;
        auto& pieces = state.player == 1 ? state.whitePieces : state.blackPieces;
        for (const auto& piece : pieces)
        {
            auto x = piece.first;
            auto y = piece.second;
            if (board.get(x + 1, y) == 0)
                result.emplace_back(x, y, Direction::east);
            if (board.get(x - 1, y) == 0)
                result.emplace_back(x, y, Direction::west);
            if (board.get(x, y + 1) == 0)
                result.emplace_back(x, y, Direction::south);
            if (board.get(x, y - 1) == 0)
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
        Drawboard board{state};

        // If it is the current player's turn, then the other player is the one
        // who may have won.
        auto player = other(state.player);

        auto pieces = player == 1 ? state.whitePieces : state.blackPieces;
        std::sort(std::begin(pieces), std::end(pieces));
        for (const auto& piece : pieces)
        {
            auto x = piece.first;
            auto y = piece.second;
            if (board.get(x + 1, y - 1) == player &&
                board.get(x + 2, y - 2) == player &&
                board.get(x + 3, y - 3) == player)
                return true;
            if (board.get(x + 1, y) == player &&
                board.get(x + 2, y) == player && board.get(x + 3, y) == player)
                return true;
            if (board.get(x + 1, y + 1) == player &&
                board.get(x + 2, y + 2) == player &&
                board.get(x + 3, y + 3) == player)
                return true;
            if (board.get(x, y + 1) == player &&
                board.get(x, y + 2) == player && board.get(x, y + 3) == player)
                return true;
        }
        return false;
    }

    int other(int player) const
    {
        return player == 1 ? 2 : 1;
    }
};

std::string to_string(const DynamicConnect4::ActionType& action)
{
    // Assume the position indicators of an action are single-digit.
    std::stringstream ss;
    ss << std::get<0>(action) << std::get<1>(action);
    switch (std::get<2>(action))
    {
    case DynamicConnect4::Direction::east:
        ss << 'E';
        break;
    case DynamicConnect4::Direction::west:
        ss << 'W';
        break;
    case DynamicConnect4::Direction::south:
        ss << 'S';
        break;
    case DynamicConnect4::Direction::north:
        ss << 'N';
        break;
    default:
        throw std::logic_error("impossible");
    }
    return ss.str();
}

std::array<std::array<int8_t, DynamicConnect4::boardSize>, DynamicConnect4::boardSize>
    DynamicConnect4::Drawboard::drawboard{0};
