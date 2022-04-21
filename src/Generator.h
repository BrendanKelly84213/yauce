#ifndef GENERATOR_H
#define GENERATOR_H

#include <cstdint>
#include <vector>
#include "BoardState.h"
#include "utils/types.h"

int in_check_generator(BoardState board_state, BMove moves[]);
int psuedo_generator(BoardState board_state, BMove moves[]);

#endif
