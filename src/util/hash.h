#pragma once

#include <functional>

namespace Util {

inline void hash_combine(size_t& /*seed*/)
{
}

// A template for combining different hashes.
template <typename T, typename... Rest>
inline void hash_combine(size_t& seed, const T& v, Rest... rest)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hash_combine(seed, rest...);
}
}
