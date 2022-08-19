#ifndef EVAL_H
#define EVAL_H 

#include "BoardState.h"

int piece_weight(PieceType pt);
int eval(const BoardState &board);
void init_black_tables();
void print_black_tables();
int pawn_chain(const BoardState &board);

#endif 
