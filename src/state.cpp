#include "state.h"

#include <vector>
#include <iostream>
#include <algorithm>

namespace DynamicConnect4 {

bool operator==(const State& lhs, const State& rhs)
{
    return lhs.isPlayerOne == rhs.isPlayerOne &&
        lhs.whitePieces == rhs.whitePieces && lhs.blackPieces == rhs.blackPieces;
}

bool operator!=(const State& lhs, const State& rhs)
{
    return !(lhs == rhs);
}

std::istream& operator>>(std::istream& in, State& state)
{
    std::vector<Point> whitePieces;
    std::vector<Point> blackPieces;

    int i = 0, j = 0;
    for (char ch; in.get(ch) && j < boardSize;)
    {
        if (ch == 'O')
        {
            if (i == boardSize)
            {
                in.clear(std::ios::failbit);
                return in;
            }
            whitePieces.emplace_back(i++, j);
        }
        else if (ch == 'X')
        {
            if (i == boardSize)
            {
                in.clear(std::ios::failbit);
                return in;
            }
            blackPieces.emplace_back(i++, j);
        }
        else if (ch == ' ')
        {
            if (i == boardSize)
            {
                in.clear(std::ios::failbit);
                return in;
            }
            ++i;
        }

        if (ch == '\n')
        {
            i = 0;
            ++j;
        }
    }

    if (whitePieces.size() != piecesPerPlayer ||
        blackPieces.size() != piecesPerPlayer)
    {
        in.clear(std::ios::failbit);
        return in;
    }

    std::sort(std::begin(whitePieces), std::end(whitePieces));
    std::copy(
        std::begin(whitePieces),
        std::end(whitePieces),
        std::begin(state.whitePieces));
    std::sort(std::begin(blackPieces), std::end(blackPieces));
    std::copy(
        std::begin(blackPieces),
        std::end(blackPieces),
        std::begin(state.blackPieces));
    state.isPlayerOne = true;

    return in;
}

std::ostream& operator<<(std::ostream& out, const State& state)
{
    out << "  1 2 3 4 5 6 7" << std::endl;
    for (int y = 0; y < boardSize; ++y)
    {
        out << (y + 1) << " ";
        for (int x = 0; x < boardSize; ++x)
        {
            if (std::find(
                    std::begin(state.whitePieces),
                    std::end(state.whitePieces),
                    Point{x, y}) != std::end(state.whitePieces))
                out << "O";
            else if (
                std::find(
                    std::begin(state.blackPieces),
                    std::end(state.blackPieces),
                    Point{x, y}) != std::end(state.blackPieces))
                out << "X";
            else
                out << " ";

            if (x + 1 < boardSize)
                out << ",";
        }
        out << std::endl;
    }
    return out;
}
}
