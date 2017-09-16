#pragma once

#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace Search {

enum class Flag
{
    exact,
    lowerBound,
    upperBound
};

template <typename Game>
class TranspositionTable
{
public:
    using StateType = typename Game::StateType;
    using EvalType = typename Game::EvalType;

    struct ValueType
    {
        EvalType value{};
        int depth{};
        Flag flag{};

        ValueType() = default;
        ValueType(EvalType value, int depth, Flag flag)
            : value{value}, depth{depth}, flag{flag}
        {
        }
    };

    using MapType = std::unordered_map<StateType, ValueType>;

    static const size_t maxSize = 8 * 1024 * 1024;

    std::pair<bool, ValueType> find(const StateType& state) const
    {
        auto entry = table.find(state);
        if (entry != std::end(table))
            return std::make_pair(true, entry->second);
        return std::make_pair(false, ValueType{});
    }

    void set(const StateType& state, EvalType value, int depth, Flag flag)
    {
        table[state] = ValueType{value, depth, flag};
        if (table.size() > maxSize)
            table.erase(std::begin(table));
    }

    size_t size() const
    {
        return table.size();
    }

private:
    MapType table;
};
}
