#include "eval.h"

const unsigned int P_weight = 100;
const unsigned int N_weight = 320;
const unsigned int B_weight = 330;
const unsigned int R_weight = 500;
const unsigned int Q_weight = 900;
const unsigned int K_weight = 20000;

// Just return the material inbalance for now
// NOTE: I am very tired and don't really gaf 
int eval(BoardState board)
{
    const size_t num_white_pawns = board.get_num_piece(WP); 
    const size_t num_white_bishops = board.get_num_piece(WB); 
    const size_t num_white_knights = board.get_num_piece(WN); 
    const size_t num_white_rooks = board.get_num_piece(WR); 
    const size_t num_white_kings = board.get_num_piece(WK); 
    const size_t num_white_queens = board.get_num_piece(WQ); 

    const size_t num_black_pawns = board.get_num_piece(BP); 
    const size_t num_black_bishops = board.get_num_piece(BB); 
    const size_t num_black_knights = board.get_num_piece(BN); 
    const size_t num_black_rooks = board.get_num_piece(BR); 
    const size_t num_black_kings = board.get_num_piece(BK); 
    const size_t num_black_queens = board.get_num_piece(BQ); 

    return P_weight * (num_white_pawns - num_black_pawns)
         + N_weight * (num_white_knights - num_black_knights)
         + B_weight * (num_white_bishops - num_black_bishops)
         + R_weight * (num_white_rooks - num_black_rooks)
         + Q_weight * (num_white_queens - num_black_queens)
         + K_weight * (num_white_kings - num_black_kings);
}
