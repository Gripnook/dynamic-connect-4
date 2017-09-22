#pragma once

#include <array>

#include "game/game.h"
#include "game/drawboard.h"

namespace DynamicConnect4 {

using StateType = Game::StateType;
using ActionType = Game::ActionType;
using EvalType = Game::EvalType;


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
}
