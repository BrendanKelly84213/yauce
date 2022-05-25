#ifndef BOARD_STATE_H
#define BOARD_STATE_H

#include "BoardState.h"

struct ScoredMove {
    BMove m;
    int score;
};

int search(BoardState board, int depth);
MoveList sorted_moves(BoardState board, int depth);

#endif
