#ifndef BOARD_STATE_H
#define BOARD_STATE_H

#include "BoardState.h"

struct ScoredMove {
    BMove m;
    int score;
    std::string alg;
};

int search(BoardState board, int depth);
void print_moves_and_scores(BoardState board, int depth);
ScoredMove best_move(BoardState board, int depth);

#endif
