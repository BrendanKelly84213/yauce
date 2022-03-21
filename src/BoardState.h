#ifndef BOARDSTATE_H
#define BOARDSTATE_H 

#include <string> 
#include <vector>
#include <iostream>
#include <cassert>
#include "utils/types.h"
#include "utils/conversions.h"

struct State {
    Colour side_to_move = White; 
    bool w_castle_ks = false; 
    bool w_castle_qs = false; 
    bool b_castle_ks = false; 
    bool b_castle_qs = false; 
    int ep_file = -1;
    int halfmove_clock = 0;
    int ply_count = 0;     
    Piece last_captured = None;

    void operator=(const State &b)
    {
        this->side_to_move    = b.side_to_move;
        this->w_castle_ks     = b.w_castle_ks;
        this->w_castle_qs     = b.w_castle_qs;
        this->b_castle_ks     = b.b_castle_ks ;
        this->b_castle_qs     = b.b_castle_qs;
        this->ep_file         = b.ep_file;
        this->halfmove_clock  = b.halfmove_clock;
        this->ply_count       = b.ply_count;
    }

    bool operator==(State &b)
    {
      return this->side_to_move   == b.side_to_move
          && this->w_castle_ks    == b.w_castle_ks
          && this->w_castle_qs    == b.w_castle_qs
          && this->b_castle_ks    == b.b_castle_ks 
          && this->b_castle_qs    == b.b_castle_qs
          && this->ep_file        == b.ep_file
          && this->halfmove_clock == b.halfmove_clock
          && this->ply_count      == b.ply_count;
    }
};

class BoardState {
private: 
    int movelist_idx = 0;
    BMove movelist[1024];
    State prev_state;
    Bitboard piece_bbs[12];
    Bitboard occ = 0ULL;
    Bitboard white_occ = 0ULL;
    Bitboard black_occ = 0ULL;

    void init_squares(std::string fen);
    void init_bbs();
    void castle_kingside();
    void uncastle_kingside();
    void castle_queenside();
    void uncastle_queenside();
    void do_castle(int rook_from, int rook_to, int king_from, int king_to);
    void move_piece(int from, int to);
    void remove_piece(int sq);
    void put_piece(int sq, Piece p);
    void update_extras();
    bool board_ok();
public: 
    
    State state;
    Piece squares[64];

    void init(std::string fen);
    void make_move(BMove m);
    void unmake_move(BMove m);
    Bitboard get_occ();
    Bitboard get_friend_occ();
    Bitboard get_friend_occ(Colour us);
    Bitboard get_op_occ();
    Bitboard get_op_occ(Colour us);
    Bitboard get_friend_piece_bb(int pt);
    Bitboard get_op_piece_bb(int pt);
    Bitboard get_side_piece_bb(int pt, Colour side);
    Colour get_piece_colour(Piece p);
    bool in_check(Colour us);
    bool attacked(int sq, Colour by);

    bool operator==(BoardState b)
    {
        for(int p=BQ; p <= WP; ++p) {
            if(this->piece_bbs[p] != b.piece_bbs[p])
                return false;
        }
        return this->white_occ == b.white_occ
            && this->black_occ == b.black_occ  
            && this->occ == b.occ
            && this->state == b.state;
    }
};

#endif
