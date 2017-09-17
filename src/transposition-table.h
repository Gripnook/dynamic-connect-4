#pragma once

#include <list>
#include <unordered_map>
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

    using ListType = std::list<std::pair<StateType, ValueType>>;
    using MapType = std::unordered_map<StateType, typename ListType::iterator>;

    static const size_t maxSize = 4 * 1024 * 1024;

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
            while (size() > maxSize)
            {
                table.erase(std::rbegin(lru)->first);
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
        return accesses == 0 ? 0.0 :
                               static_cast<double>(accesses - misses) / accesses;
    }

private:
    MapType table;
    ListType lru;

    mutable int accesses{0};
    mutable int misses{0};
};
}
