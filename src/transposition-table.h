#pragma once

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <functional>

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
    using SetType = std::unordered_set<StateType>;

    void reset()
    {
        table.clear();
    }

    std::pair<bool, ValueType> find(const StateType& state) const
    {
        auto entry = table.find(state);
        if (entry != std::end(table))
            return std::make_pair(true, entry->second);

        if (playerOneWins.count(state) > 0)
            return std::make_pair(
                true,
                ValueType{std::numeric_limits<EvalType>::max(), 0, Flag::exact});

        if (playerTwoWins.count(state) > 0)
            return std::make_pair(
                true,
                ValueType{std::numeric_limits<EvalType>::lowest(),
                          0,
                          Flag::exact});

        return std::make_pair(false, ValueType{});
    }

    void set(const StateType& state, EvalType value, int depth, Flag flag)
    {
        if (value == std::numeric_limits<EvalType>::max())
            playerOneWins.insert(state);
        else if (value == std::numeric_limits<EvalType>::lowest())
            playerTwoWins.insert(state);
        else
            table[state] = ValueType{value, depth, flag};
    }

    size_t size() const
    {
        return table.size() + playerOneWins.size() + playerTwoWins.size();
    }

private:
    MapType table;
    SetType playerOneWins;
    SetType playerTwoWins;
};
}
