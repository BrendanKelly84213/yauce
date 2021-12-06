#ifndef GENERATOR_H
#define GENERATOR_H

#include <cstdint>
#include <vector>
#include "BoardState.h"

typedef uint64_t Bitboard;
typedef uint16_t BMove;

std::vector<Move> boardstate_to_move_vec(BoardState board_state);
void init_generator();
Bitboard get_piece_moves(PieceType p, Square from, Bitboard occ);
void print(Bitboard bb); 
PieceType piece_to_piecetype(Piece piece); 
Bitboard occ_squares(Piece* squares, Colour colour);
Bitboard get_behind(Square from, Square to);
inline Square pop_bit(Bitboard &bb);
std::string get_square_to_str(Square s);


#endif
