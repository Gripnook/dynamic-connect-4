#pragma once

#include <unordered_map>
#include <utility>
#include <functional>

namespace Search {

enum class Flag
{
    exact,
    lowerBound,
    upperBound
};

// A cache for storing game states during search. The cache has two layers.
// The local layer acts as the main store and can be reset between searches to
// avoid overusing memory. The global layer is persistent and can be used to
// store special states, as given by the StorageCriteria.
// Game::StateType must be hashable (std::hash) and comparable (==).
template <typename Game>
class StateCache
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

    using StorageCriteria =
        std::function<bool(const StateType&, EvalType, Flag)>;

    StateCache(StorageCriteria criteria) : criteria{criteria}
    {
    }

    void reset()
    {
        localCache.clear();
    }

    std::pair<bool, ValueType> find(const StateType& state) const
    {
        auto entry = globalCache.find(state);
        if (entry != std::end(globalCache))
            return std::make_pair(true, entry->second);

        entry = localCache.find(state);
        if (entry != std::end(localCache))
            return std::make_pair(true, entry->second);

        return std::make_pair(false, ValueType{});
    }

    void set(const StateType& state, EvalType value, int depth, Flag flag)
    {
        if (criteria(state, value, flag))
            globalCache[state] = ValueType{value, depth, flag};
        else
            localCache[state] = ValueType{value, depth, flag};
    }

    size_t size() const
    {
        return localCacheSize() + globalCacheSize();
    }

    size_t localCacheSize() const
    {
        return localCache.size();
    }

    size_t globalCacheSize() const
    {
        return globalCache.size();
    }

private:
    MapType globalCache;
    MapType localCache;

    StorageCriteria criteria;
};
}
