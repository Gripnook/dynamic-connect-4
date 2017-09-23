#pragma once

#include <cstdint>

namespace DynamicConnect4 {

static const int boardSize = 7;
static const int piecesPerPlayer = 6;

enum class Direction : int8_t
{
    east,
    west,
    south,
    north
};
}
