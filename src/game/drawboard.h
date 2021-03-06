#pragma once

#include <array>
#include <cstdint>

#include "game/definition.h"
#include "game/point.h"
#include "game/state.h"

namespace DynamicConnect4 {

// A small static array representing the game board to use as scratch paper when
// computing things. The constructor and destructor fill and erase the array, so
// care must be taken to avoid multiple simultaneous instantiations.
class Drawboard
{
public:
    Drawboard(State state) : state{state}
    {
        for (const auto& piece : state.whitePieces)
            drawboard[piece.x()][piece.y()] = 1;
        for (const auto& piece : state.blackPieces)
            drawboard[piece.x()][piece.y()] = 2;
    }

    ~Drawboard()
    {
        for (const auto& piece : state.whitePieces)
            drawboard[piece.x()][piece.y()] = 0;
        for (const auto& piece : state.blackPieces)
            drawboard[piece.x()][piece.y()] = 0;
    }

    int8_t get(int x, int y) const
    {
        if (x < 0 || x >= boardSize || y < 0 || y >= boardSize)
            return -1;
        return drawboard[x][y];
    }

private:
    static std::array<std::array<int8_t, boardSize>, boardSize> drawboard;

    State state;
};
}
