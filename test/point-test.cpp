#include "catch.hpp"

#include "game/point.h"

#include <vector>
#include <algorithm>

TEST_CASE("Points behave like a pair of integers")
{
    Point p1{3, 2};
    CHECK(p1.x() == 3);
    CHECK(p1.y() == 2);

    p1.set(0, 15);
    CHECK(p1.x() == 0);
    CHECK(p1.y() == 15);
}

TEST_CASE("Points can be compared for equality and inequality")
{
    Point p1{15, 0};
    Point p2{15, 0};
    Point p3{4, 5};
    CHECK(p1 == p2);
    CHECK(p1 != p3);
}

TEST_CASE("Points can be ordered first by x and then y")
{
    std::vector<Point> v;
    v.emplace_back(3, 2);
    v.emplace_back(6, 1);
    v.emplace_back(15, 7);
    v.emplace_back(15, 4);
    v.emplace_back(4, 8);

    std::sort(std::begin(v), std::end(v));
    REQUIRE(std::is_sorted(std::begin(v), std::end(v), [](Point lhs, Point rhs) {
        return lhs.x() == rhs.x() ? lhs.y() < rhs.y() : lhs.x() < rhs.x();
    }));
}

TEST_CASE("Default constructed points are equal to 0")
{
    Point p1 = Point{};
    Point p2 = Point{0, 0};
    REQUIRE(p1 == p2);
}
