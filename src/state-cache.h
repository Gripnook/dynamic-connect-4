#pragma once

#include <unordered_map>
#include <utility>
#include <functional>

namespace Search {

// A cache for storing game states during search. The cache has two layers. The
// local layer acts as the main store and can be reset between searches to
// avoid overusing memory. The global layer is persistent and can be used to
// store special states, as given by the StorageCriteria.
// Game::StateType must be hashable (std::hash) and comparable (==).
template <typename Game>
class StateCache
{
public:
    using StateType = typename Game::StateType;
    using EvalType = typename Game::EvalType;

    using MapType = std::unordered_map<StateType, std::pair<EvalType, int>>;

    using StorageCriteria = std::function<bool(const StateType&, EvalType)>;

    StateCache(StorageCriteria criteria) : criteria{criteria}
    {
    }

    void reset()
    {
        localCache.clear();
    }

    std::pair<bool, EvalType> find(const StateType& state, int depth) const
    {
        // Entries in the global cache are valid regardless of depth.
        auto entry = globalCache.find(state);
        if (entry != std::end(globalCache))
            return std::make_pair(true, entry->second.first);
        // Entries in the local cache must be depth checked.
        entry = localCache.find(state);
        if (entry != std::end(localCache) && entry->second.second <= depth)
            return std::make_pair(true, entry->second.first);
        return std::make_pair(false, 0);
    }

    void set(const StateType& state, EvalType value, int depth)
    {
        if (criteria(state, value))
        {
            globalCache[state] = std::make_pair(value, depth);
        }
        else
        {
            auto entry = localCache.emplace(state, std::make_pair(value, depth));
            if (!entry.second)
            {
                int entryDepth = entry.first->second.second;
                if (entryDepth > depth)
                    entry.first->second = std::make_pair(value, depth);
            }
        }
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
