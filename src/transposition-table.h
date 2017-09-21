#pragma once

#include <list>
#include <unordered_map>
#include <utility>
#include <map>

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
    using DepthIteratorMapType = std::map<int, typename ListType::iterator>;

    static const size_t lruMaxSize = 2 * 1024 * 1024;
    static const size_t depthMaxSize = 2 * 1024 * 1024;

    std::pair<bool, ValueType> find(const StateType& state)
    {
        ++accesses;
        auto entry = lruTable.find(state);
        if (entry != std::end(lruTable))
        {
            lruList.splice(std::begin(lruList), lruList, entry->second);
            return std::make_pair(true, entry->second->second);
        }
        entry = depthTable.find(state);
        if (entry != std::end(depthTable))
        {
            return std::make_pair(true, entry->second->second);
        }
        ++misses;
        return std::make_pair(false, ValueType{});
    }

    void emplace(const StateType& state, EvalType value, int depth, Flag flag)
    {
        lruEmplace(state, value, depth, flag);
        depthEmplace(state, value, depth, flag);
    }

    void expire()
    {
        depthExpire();
    }

    void clear()
    {
        lruTable.clear();
        lruList.clear();
        depthTable.clear();
        depthList.clear();
        depthIterators.clear();
    }

    size_t size() const
    {
        return lruTable.size() + depthTable.size();
    }

    double getHitRate() const
    {
        return accesses == 0 ? 0.0 :
                               static_cast<double>(accesses - misses) / accesses;
    }

private:
    MapType lruTable;
    ListType lruList;

    MapType depthTable;
    ListType depthList;
    DepthIteratorMapType depthIterators;

    mutable int accesses{0};
    mutable int misses{0};

    void lruEmplace(const StateType& state, EvalType value, int depth, Flag flag)
    {
        auto entry = lruTable.find(state);
        if (entry != std::end(lruTable))
        {
            lruList.splice(std::begin(lruList), lruList, entry->second);
            entry->second->second = ValueType{value, depth, flag};
        }
        else
        {
            lruList.emplace_front(state, ValueType{value, depth, flag});
            lruTable[state] = std::begin(lruList);
            while (lruTable.size() > lruMaxSize)
            {
                lruTable.erase(lruList.back().first);
                lruList.pop_back();
            }
        }
    }

    void
        depthEmplace(const StateType& state, EvalType value, int depth, Flag flag)
    {
        auto entry = depthTable.find(state);
        if (entry != std::end(depthTable))
        {
            entry->second->second = ValueType{value, depth, flag};
            depthMaintain(depth, entry->second);
        }
        else
        {
            depthList.emplace_back(state, ValueType{value, depth, flag});
            depthTable[state] = --std::end(depthList);
            depthMaintain(depth, --std::end(depthList));
            while (depthTable.size() > depthMaxSize)
            {
                for (auto& it : depthIterators)
                {
                    if (it.second != std::begin(depthList))
                        break;
                    ++it.second;
                }
                depthTable.erase(depthList.front().first);
                depthList.pop_front();
            }
        }
    }

    void depthMaintain(int depth, typename ListType::iterator listIt)
    {
        auto it = depthIterators.find(depth + 1);
        if (it != std::end(depthIterators))
        {
            depthList.splice(it->second, depthList, listIt);
        }
        else
        {
            depthList.splice(std::end(depthList), depthList, listIt);
            it = depthIterators.find(depth);
            if (it == std::end(depthIterators))
                depthIterators.emplace(depth, --std::end(depthList));
        }
    }

    void depthExpire()
    {
        if (std::begin(depthIterators) == std::end(depthIterators))
            return;
        auto it1 = std::begin(depthIterators);
        auto it2 = ++std::begin(depthIterators);
        while (it2 != std::end(depthIterators))
        {
            it1->second = it2->second;
            ++it1;
            ++it2;
        }
        depthIterators.erase(--std::end(depthIterators));
    }
};
}
