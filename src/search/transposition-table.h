#pragma once

#include <list>
#include <unordered_map>
#include <utility>
#include <cstdint>

namespace Search {

enum class Flag : int8_t
{
    exact,
    lowerBound,
    upperBound
};

// A class implementing a transposition table for a game of type Game.
// This table takes the form of a fixed size, LRU replacement hash table which
// is used to store states, their values, the depths at which their values were
// computed, and the types of values stored (EXACT, LOWER_BOUND, UPPER_BOUND).
//
// Game must define:
//      StateType - The type of the state representation for a position.
//          This type must be hashable using std::hash<StateType> and
//          comparable for equality using the == operator.
//      EvalType - The type of a numerical position evaluation.
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

    TranspositionTable(size_t maxSize) : maxSize{maxSize}
    {
    }

    std::pair<bool, ValueType> find(const StateType& state)
    {
        ++accesses;
        auto entry = table.find(state);
        if (entry != std::end(table))
        {
            lru.splice(std::begin(lru), lru, entry->second);
            return std::make_pair(true, entry->second->second);
        }
        ++misses;
        return std::make_pair(false, ValueType{});
    }

    void emplace(const StateType& state, EvalType value, int depth, Flag flag)
    {
        auto entry = table.find(state);
        if (entry != std::end(table))
        {
            lru.splice(std::begin(lru), lru, entry->second);
            entry->second->second = ValueType{value, depth, flag};
        }
        else
        {
            lru.emplace_front(state, ValueType{value, depth, flag});
            table[state] = std::begin(lru);
            while (table.size() > maxSize)
            {
                table.erase(lru.back().first);
                lru.pop_back();
            }
        }
    }

    void clear()
    {
        table.clear();
        lru.clear();
    }

    size_t size() const
    {
        return table.size();
    }

    double getHitRate() const
    {
        if (accesses == 0)
            return 0.0;
        return static_cast<double>(accesses - misses) / accesses;
    }

private:
    using ListType = std::list<std::pair<StateType, ValueType>>;
    using MapType = std::unordered_map<StateType, typename ListType::iterator>;

    MapType table;
    ListType lru;

    size_t maxSize{};

    mutable int accesses{0};
    mutable int misses{0};
};
}
