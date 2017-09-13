#pragma once

#include <array>

#include "game/definition.h"
#include "game/point.h"

namespace DynamicConnect4 {

// We store only the piece locations for efficiency.
// The arrays are assumed to be kept sorted.
struct State
{
    bool isPlayerOne{true};
    std::array<Point, piecesPerPlayer> whitePieces{{Point{0, 2},
                                                    Point{0, 4},
                                                    Point{0, 6},
                                                    Point{6, 1},
                                                    Point{6, 3},
                                                    Point{6, 5}}};
    std::array<Point, piecesPerPlayer> blackPieces{{Point{0, 1},
                                                    Point{0, 3},
                                                    Point{0, 5},
                                                    Point{6, 0},
                                                    Point{6, 2},
                                                    Point{6, 4}}};
};

bool operator==(const State& lhs, const State& rhs)
{
    return lhs.isPlayerOne == rhs.isPlayerOne &&
        lhs.whitePieces == rhs.whitePieces && lhs.blackPieces == rhs.blackPieces;
}

bool operator!=(const State& lhs, const State& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const State& lhs, const State& rhs)
{

    if (lhs.isPlayerOne != rhs.isPlayerOne)
        return lhs.isPlayerOne < rhs.isPlayerOne;
    if (lhs.whitePieces != rhs.whitePieces)
        return lhs.whitePieces < rhs.whitePieces;
    return lhs.blackPieces < rhs.blackPieces;
}
}
