#pragma once

#include <vector>
#include <utility>
#include <iosfwd>

#include "game/definition.h"
#include "game/point.h"
#include "game/state.h"

namespace DynamicConnect4 {

class Game
{
public:
    using StateType = State;
    using ActionType = std::pair<Point, Direction>;
    using EvalType = float;

    std::vector<ActionType> getActions(const StateType& state) const;
    StateType getResult(StateType state, const ActionType& action) const;
    bool isTerminal(const StateType& state) const;
    EvalType getUtility(const StateType& state) const;
};

std::istream& operator>>(std::istream& in, Game::ActionType& action);
std::ostream& operator<<(std::ostream& out, const Game::ActionType& action);
}
