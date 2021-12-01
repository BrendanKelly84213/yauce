#ifndef GENERATOR_H
#define GENERATOR_H

#include <cstdint>
#include <vector>
#include "BoardState.h"

typedef uint64_t Bitboard;
typedef uint16_t BMove;

std::vector<Move> boardstate_to_move_vec(BoardState board_state);


#endif
