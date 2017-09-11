#pragma once

using StateType = typename DynamicConnect4::StateType;
using ActionType = typename DynamicConnect4::ActionType;
using EvalType = typename DynamicConnect4::EvalType;

class Heuristic1
{
public:
    EvalType operator()(const StateType& state)
    {
        return evalRows(1, state) + evalColumns(1, state) - evalRows(2, state) -
            evalColumns(2, state);
    }

private:
    EvalType evalRows(int32_t player, const StateType& state) const
    {
        EvalType result = 0;
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (size_t row = 0; row < DynamicConnect4::boardSize; ++row)
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
        for (size_t col = 0; col < DynamicConnect4::boardSize; ++col)
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
};

class Heuristic2
{
public:
    EvalType operator()(const StateType& state)
    {
        return evalRows(1, state) + evalColumns(1, state) +
            evalDiameter(1, state) - evalRows(2, state) -
            evalColumns(2, state) - evalDiameter(2, state);
    }

private:
    EvalType evalRows(int32_t player, const StateType& state) const
    {
        EvalType result = 0;
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (size_t row = 0; row < DynamicConnect4::boardSize; ++row)
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
                    {
                        ++count;
                        result += count;
                    }
                    else
                    {
                        count = 0;
                        result += (DynamicConnect4::boardSize - col);
                    }
                });
        }
        return result;
    }

    EvalType evalColumns(int32_t player, const StateType& state) const
    {
        EvalType result = 0;
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;
        for (size_t col = 0; col < DynamicConnect4::boardSize; ++col)
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
                    {
                        ++count;
                        result += count;
                    }
                    else
                    {
                        count = 0;
                        result += (DynamicConnect4::boardSize - row);
                    }
                });
        }
        return result;
    }

    EvalType evalDiameter(int32_t player, const StateType& state) const
    {
        EvalType result = 0;
        auto& pieces = player == 1 ? state.whitePieces : state.blackPieces;

        for (auto it1 = std::begin(pieces); it1 != std::end(pieces); ++it1)
        {
            for (auto it2 = it1 + 1; it2 != std::end(pieces); ++it2)
            {
                for (auto it3 = it2 + 1; it3 != std::end(pieces); ++it3)
                {
                    for (auto it4 = it3 + 1; it4 != std::end(pieces); ++it4)
                    {
                        std::vector<std::pair<int32_t, int32_t>> v{*it1,
                                                                   *it2,
                                                                   *it3,
                                                                   *it4};
                        std::sort(v.begin(), v.end());
                        EvalType temp = 0;
                        for (auto it = v.begin() + 1; it != v.end(); ++it)
                            temp += eval(it - 1, it);
                        // temp += eval(it1, it2);
                        // temp += eval(it1, it3);
                        // temp += eval(it1, it4);
                        // temp += eval(it2, it3);
                        // temp += eval(it2, it4);
                        // temp += eval(it3, it4);
                        result = std::max(result, temp);
                    }
                }
            }
        }
        return result;
    }

    template <typename It>
    EvalType eval(It it1, It it2) const
    {
        return 2 * DynamicConnect4::boardSize -
            std::abs(
                   static_cast<EvalType>(it1->first) -
                   static_cast<EvalType>(it2->first)) -
            std::abs(
                   static_cast<EvalType>(it1->second) -
                   static_cast<EvalType>(it2->second));
    }
};
