#pragma once

#include <array>
#include <iostream>

#include "heuristics.h"

class Agent
{
public:
    using HeuristicFunc =
        Heuristic<ConsecutiveElements, NearbyElements, Proximity, CentralDomination>;

    void startGame(int32_t player)
    {
        started = true;
        this->player = player;

        randomIndex = rand() % 4;
        randomShift =
            weights[randomIndex] * 0.2 * ((rand() % 100000) / (100000.0) - 1.0);
        weights[randomIndex] += randomShift;

        std::cout << "( ";
        for (auto weight : weights)
            std::cout << weight << " ";
        std::cout << ")" << std::endl;
    }

    void endGame(EvalType utility, int32_t moveCount)
    {
        if (!started)
            throw std::logic_error("inactive agent cannot be stopped");
        started = false;

        if ((player == 1 && utility > 0) || (player == 2 && utility < 0))
        {
            if (moveCount > bestMoveCount)
            {
                weights[randomIndex] -= randomShift;
            }
            else
            {
                bestMoveCount = moveCount;
                std::cout << "win in " << moveCount << " moves" << std::endl;
            }
        }
        else
        {
            weights[randomIndex] -= randomShift;
        }
    }

    std::function<EvalType(const StateType&)> getHeuristic()
    {
        return HeuristicFunc{weights[0], weights[1], weights[2], weights[3]};
    }

private:
    std::array<EvalType, 4> weights{{1.0, 1.0, 1.0, 1.0}};
    size_t randomIndex;
    EvalType randomShift;
    int32_t bestMoveCount{100};

    bool started{false};
    int32_t player;
};
