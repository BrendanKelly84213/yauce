#ifndef BOARDSTATE_H
#define BOARDSTATE_H 

#include <string> 
#include <iostream>
#include "utils/types.h"
#include "utils/conversions.h"

class BoardState {
private: 
    void init_squares(std::string fen);
    void init_bbs();
    void castle_kingside();
    void castle_queenside();
    void do_castle(int rook_from, int rook_to, int king_to);
public: 
    Bitboard piece_bbs[12];
    Bitboard occ;
    Bitboard white_occ;
    Bitboard black_occ;
    Bitboard white_piece_bbs[6];
    Bitboard black_piece_bbs[6];

    Piece squares[64];
    Colour side_to_move; 
    bool w_castle_ks = false; 
    bool w_castle_qs = false; 
    bool b_castle_ks = false; 
    bool b_castle_qs = false; 
    int ep_file;
    int halfmove_clock;
    int ply_count; 

    void init(std::string fen);
    void make_move(BMove m);
    Bitboard get_friend_occ();
    Bitboard get_op_occ();
    Bitboard get_friend_piece_bb(int pt);
    Bitboard get_op_piece_bb(int pt);
};

#endif
