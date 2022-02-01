#ifndef GENERATOR_H
#define GENERATOR_H

#include <cstdint>
#include <vector>
#include "BoardState.h"
#include "utils/types.h"

typedef uint64_t Bitboard;
typedef uint16_t BMove;

BMove* generator(Bitboard * piece_bbs, 
        BoardState board_state, 
        Bitboard friend_occ, 
        Bitboard op_occ);
void init_generator();
Bitboard blockers_and_beyond(int p, int from, Bitboard occ);
Bitboard get_to_squares(int p, int from, BoardState board_state);
void print(Bitboard bb); 
PieceType piece_to_piecetype(Piece piece); 
Bitboard occ_squares(Piece* squares, Colour colour);
Bitboard get_behind(Square from, Square to);
inline Square pop_bit(Bitboard &bb);
std::string get_square_to_str(Square s);


#endif
