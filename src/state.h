#pragma once

#include <array>
#include <iosfwd>
#include <functional>
#include <cstdint>

#include "definition.h"
#include "point.h"
#include "hash.h"

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

bool operator==(const State& lhs, const State& rhs);
bool operator!=(const State& lhs, const State& rhs);

std::istream& operator>>(std::istream& in, State& state);
std::ostream& operator<<(std::ostream& out, const State& state);
}

namespace std {
template <>
struct hash<DynamicConnect4::State>
{
    size_t operator()(const DynamicConnect4::State& state) const
    {
        // We reinterpret the pointer to the state to unpack the bits of the
        // representation 64 bits at a time. This is more efficient than the
        // loop approach, which helps the search run deeper.

        static_assert(
            sizeof(state) == 13 * sizeof(uint8_t), "cannot hash state");

        auto ptr = reinterpret_cast<const uint8_t*>(&state);
        auto firstPart = *reinterpret_cast<const uint64_t*>(ptr);
        auto secondPart = *reinterpret_cast<const uint64_t*>(ptr + 5);

        size_t seed = 0;
        Util::hash_combine(seed, firstPart, secondPart);
        return seed;
    }
};
}
