#include <boost/python.hpp>

#include "game/game.h"
#include "search/iterative-alpha-beta.h"

using namespace DynamicConnect4;
using namespace Search;

using StateType = Game::StateType;
using ActionType = Game::ActionType;
using EvalType = Game::EvalType;

Game game{};
IterativeAlphaBeta<Game> searcher{game};

void printState(const StateType& state);
void printAction(const ActionType& action);
ActionType search(const StateType& state, int timeLimitInMs, bool isMax);

BOOST_PYTHON_MODULE(dc4)
{
    using namespace boost::python;
    class_<ActionType>("Action");
    class_<StateType>("State");
    class_<Game>("Game")
        .def("getResult", &Game::getResult)
        .def("isTerminal", &Game::isTerminal)
        .def("getUtility", &Game::getUtility);
    def("printState", &printState);
    def("printAction", &printAction);
    def("search", search);
}

void printState(const StateType& state)
{
    std::cout << state;
}

void printAction(const ActionType& action)
{
    std::cout << action << std::endl;
}

ActionType search(const StateType& state, int timeLimitInMs, bool isMax)
{
    return searcher.search(
        state,
        [](const StateType& state) { return 0.0; },
        timeLimitInMs,
        isMax);
}
