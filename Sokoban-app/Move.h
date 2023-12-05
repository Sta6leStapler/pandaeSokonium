#pragma once

#include "GameBoard.h"

class Move {
public:
    int row;
    int col;
    int direction;
    Move(int r, int c, int d);
};

class MoveStack {
private:
    std::vector<Move> moves;
public:
    void addMove(int row, int col, int direction);
    bool isEmpty();
    void undoLastMove(GameBoard& board);
};