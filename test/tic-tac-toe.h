#pragma once

#include <vector>
#include <array>
#include <utility>

class TicTacToe
{
public:
    static const int boardSize = 3;

    struct StateType
    {
        int player{1};
        std::array<std::array<int, boardSize>, boardSize> board{};
    };

    using ActionType = std::pair<int, int>;
    using EvalType = double;

    std::vector<ActionType> getActions(const StateType& state) const
    {
        std::vector<ActionType> result;
        for (int i = 0; i < boardSize; ++i)
            for (int j = 0; j < boardSize; ++j)
                if (state.board[i][j] == 0)
                    result.emplace_back(i, j);
        return result;
    }

    StateType getResult(StateType state, const ActionType& action) const
    {
        state.board[action.first][action.second] = state.player;
        state.player = other(state.player);
        return state;
    }

    bool isTerminal(const StateType& state) const
    {
        if (getActions(state).empty())
            return true;
        return isWinner(state);
    }

    EvalType getUtility(const StateType& state) const
    {
        // Assume state is terminal.
        if (isWinner(state))
            return state.player == 1 ? -1 : 1;
        return 0;
    }

private:
    bool isWinner(const StateType& state) const
    {
        // If it is the current player's turn, then the other player is the one
        // who may have won.
        auto player = other(state.player);

        return checkRows(player, state) || checkColumns(player, state) ||
            checkDiagonal(player, state) || checkAntiDiagonal(player, state);
    }

    bool checkRows(int player, const StateType& state) const
    {
        for (int i = 0; i < boardSize; ++i)
            if (checkRow(player, state, i))
                return true;
        return false;
    }

    bool checkRow(int player, const StateType& state, int row) const
    {
        for (int i = 0; i < boardSize; ++i)
            if (state.board[row][i] != player)
                return false;
        return true;
    }

    bool checkColumns(int player, const StateType& state) const
    {
        for (int i = 0; i < boardSize; ++i)
            if (checkColumn(player, state, i))
                return true;
        return false;
    }

    bool checkColumn(int player, const StateType& state, int col) const
    {
        for (int i = 0; i < boardSize; ++i)
            if (state.board[i][col] != player)
                return false;
        return true;
    }

    bool checkDiagonal(int player, const StateType& state) const
    {
        for (int i = 0; i < boardSize; ++i)
            if (state.board[i][i] != player)
                return false;
        return true;
    }

    bool checkAntiDiagonal(int player, const StateType& state) const
    {
        for (int i = 0; i < boardSize; ++i)
            if (state.board[i][boardSize - 1 - i] != player)
                return false;
        return true;
    }

    int other(int player) const
    {
        return player == 1 ? 2 : 1;
    }
};
