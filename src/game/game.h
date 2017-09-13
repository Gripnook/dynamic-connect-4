#pragma once

#include <vector>
#include <utility>
#include <algorithm>
#include <limits>
#include <string>
#include <sstream>

#include "game/definition.h"
#include "game/point.h"
#include "game/state.h"
#include "game/drawboard.h"

namespace DynamicConnect4 {

class Game
{
public:
    using StateType = State;
    using ActionType = std::pair<Point, Direction>;
    using EvalType = float;

    std::vector<ActionType> getActions(const StateType& state) const
    {
        Drawboard board{state};

        std::vector<ActionType> result;
        auto& pieces = state.isPlayerOne ? state.whitePieces : state.blackPieces;
        for (const auto& piece : pieces)
        {
            auto x = piece.x();
            auto y = piece.y();
            if (board.get(x + 1, y) == 0)
                result.emplace_back(Point{x, y}, Direction::east);
            if (board.get(x - 1, y) == 0)
                result.emplace_back(Point{x, y}, Direction::west);
            if (board.get(x, y + 1) == 0)
                result.emplace_back(Point{x, y}, Direction::south);
            if (board.get(x, y - 1) == 0)
                result.emplace_back(Point{x, y}, Direction::north);
        }
        return result;
    }

    StateType getResult(StateType state, const ActionType& action) const
    {
        auto x = action.first.x();
        auto y = action.first.y();
        auto direction = action.second;

        auto& pieces = state.isPlayerOne ? state.whitePieces : state.blackPieces;
        auto it =
            std::lower_bound(std::begin(pieces), std::end(pieces), Point{x, y});
        switch (direction)
        {
        case Direction::east:
            it->set(x + 1, y);
            break;
        case Direction::west:
            it->set(x - 1, y);
            break;
        case Direction::south:
            it->set(x, y + 1);
            break;
        case Direction::north:
            it->set(x, y - 1);
            break;
        default:
            throw std::logic_error("impossible");
        }
        // Very important to sort after insertion.
        std::sort(std::begin(pieces), std::end(pieces));
        state.isPlayerOne = !state.isPlayerOne;
        return state;
    }

    bool isTerminal(const StateType& state) const
    {
        Drawboard board{state};

        // If it is the current player's turn,
        // then the other player is the one who may have won.
        bool isPlayerOneWinner = !state.isPlayerOne;
        int player = isPlayerOneWinner ? 1 : 2;

        // Since the pieces are sorted, we only need to look forward in the row,
        // column, diagonal, and antidiagonal for each piece.
        auto& pieces = isPlayerOneWinner ? state.whitePieces : state.blackPieces;
        for (const auto& piece : pieces)
        {
            auto x = piece.x();
            auto y = piece.y();
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

    EvalType getUtility(const StateType& state) const
    {
        // Assume the state is terminal. If it is the current player's turn,
        // then the other player is the winner.
        return state.isPlayerOne ? std::numeric_limits<EvalType>::lowest() :
                                   std::numeric_limits<EvalType>::max();
    }
};

std::string to_string(const Game::ActionType& action)
{
    // Assume the position indicators of an action are single-digit.
    std::stringstream ss;
    ss << action.first.x() + 1 << action.first.y() + 1;
    switch (action.second)
    {
    case Direction::east:
        ss << 'E';
        break;
    case Direction::west:
        ss << 'W';
        break;
    case Direction::south:
        ss << 'S';
        break;
    case Direction::north:
        ss << 'N';
        break;
    default:
        throw std::logic_error("impossible");
    }
    return ss.str();
}
}
