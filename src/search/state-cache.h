#pragma once

#include <map>
#include <utility>
#include <functional>

namespace Search {

// A cache for storing game states during search. The cache has two layers. The
// local layer acts as the main store and can be reset between searches to
// avoid overusing memory. The global layer is persistent and can be used to
// store special states, as given by the StorageCriteria.
// Game::StateType must be comparable (== and <).
template <typename Game>
class StateCache
{
public:
    using StateType = typename Game::StateType;
    using EvalType = typename Game::EvalType;

    using StorageCriteria = std::function<bool(const StateType&, EvalType)>;

    StateCache(StorageCriteria criteria) : criteria{criteria}
    {
    }

    void reset()
    {
        localCache = std::map<StateType, EvalType>{};
        cachedEntry = std::pair<StateType, EvalType>{};
    }

    bool contains(const StateType& state) const
    {
        return find(state);
    }

    EvalType get(const StateType& state) const
    {
        if (state == cachedEntry.first || find(state))
            return cachedEntry.second;
        throw std::runtime_error("entry not found");
    }

    void set(const StateType& state, EvalType value)
    {
        if (criteria(state, value))
            globalCache[state] = value;
        else
            localCache[state] = value;
        cachedEntry = std::pair<StateType, EvalType>{};
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
    std::map<StateType, EvalType> globalCache;
    std::map<StateType, EvalType> localCache;

    StorageCriteria criteria;

    mutable std::pair<StateType, EvalType> cachedEntry;

    // Looks up the entry and caches it if it finds it.
    // The caching is useful since we usually check if an entry is present
    // then get it in different calls.
    bool find(const StateType& state) const
    {
        auto entry = localCache.find(state);
        if (entry != std::end(localCache))
        {
            cachedEntry = *entry;
            return true;
        }
        entry = globalCache.find(state);
        if (entry != std::end(globalCache))
        {
            cachedEntry = *entry;
            return true;
        }
        return false;
    }
};
}
