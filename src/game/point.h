#pragma once

#include <cstdint>

namespace DynamicConnect4 {

// A space efficient representation of the position of a piece on the board.
// This class assumes that the coordinates satisfy:
//         0 <= x < 16 and 0 <= y < 16
class Point
{
public:
    Point() = default;

    Point(int x, int y) : position{static_cast<uint8_t>((x << 4) | y)}
    {
    }

    void set(int x, int y)
    {
        position = static_cast<uint8_t>((x << 4) | y);
    }

    int x() const
    {
        return position >> 4;
    }

    int y() const
    {
        return position & 0x0F;
    }

    bool operator==(Point rhs) const
    {
        return position == rhs.position;
    }

    bool operator!=(Point rhs) const
    {
        return !(*this == rhs);
    }

    bool operator<(Point rhs) const
    {
        return position < rhs.position;
    }

private:
    uint8_t position{};
};
}
