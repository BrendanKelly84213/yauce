#ifndef BOARDSTATE_H
#define BOARDSTATE_H 

#include <string> 
#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>
#include "utils/types.h"
#include "utils/conversions.h"
#include "utils/bits.h"
#include "zobrist.h"

// TODO: Attacks and related methods really belong in their own class...
// Sliding piece lookup tables 
static Bitboard piece_attacks[6][64];
static Bitboard behind[64][64];

#if 1
inline Bitboard get_piece_attacks(PieceType pt, Square from)
{
    return piece_attacks[pt][from];
}

inline Bitboard get_behind(Square from, Square to)
{
    return behind[from][to];
}

// the moves of a sliding piece up until a square, including that square 
inline Bitboard get_sliding_until_i(PieceType pt, Square from, Square to)
{
   return  get_piece_attacks(pt, from) & ~get_behind(from, to);
}

// the moves of a sliding piece up until a square, not including that square 
inline Bitboard get_sliding_until_ni(PieceType pt, Square from, Square to)
{
   return get_sliding_until_i(pt, from, to) & ~bit(to);
}
#endif 

struct State {
    Colour side_to_move; 
    // TODO: Castle can be a lot more lightweight...
    bool w_castle_ks; 
    bool w_castle_qs; 
    bool b_castle_ks; 
    bool b_castle_qs; 
    Square ep_square;
    size_t halfmove_clock;
    size_t ply_count;     
    Piece last_captured;
    bool endgame;
    bool repitition; // number of board repititions
    size_t last_irreversable; // number of board repititions
};

class BoardState {
public: 

    void init(std::string fen);

    void make_move(BMove m);
    void unmake_move(BMove m);

    void make_null_move();
    void unmake_null_move();

    ZobristKey get_hash() const { return history.back(); }
    ZobristKey generate_key_after_move(BMove m) const;
    bool is_repitition() const;
    Bitboard get_occ() const { return occ; }
    Bitboard get_friend_occ() const;
    Bitboard get_friend_occ(Colour us) const;
    Bitboard get_op_occ() const;
    Bitboard get_op_occ(Colour us) const;
    Bitboard get_piece_bb(Piece p) const { return piece_bbs[p]; }
    Bitboard get_piece_bb(PieceType p, Colour us) const { return piece_bbs[piecetype_to_piece(p, us)]; }
    Bitboard get_piecetype_bb(PieceType pt) const { return piece_bbs[piecetype_to_piece(pt, White)] | piece_bbs[piecetype_to_piece(pt, Black)]; }
    Bitboard get_friend_piece_bb(int pt) const;
    Bitboard get_op_piece_bb(int pt) const;
    Bitboard get_side_piece_bb(int pt, Colour side) const;
    Bitboard get_to_squares(PieceType pt, Square from, Colour us) const;
    Bitboard checkers(Colour us) const; 
    Colour get_piece_colour(Piece p) const;
    Colour get_side_to_move() const { return state.side_to_move; }
    Square get_ep_square() const;
    Square get_king_square(Colour us) const;
    Piece get_piece(Square s) const;
    static int get_opposite_end(Colour us) { return us == White ? 7 : 0; }

    size_t get_num_piece(Piece p) const { return popcount(piece_bbs[p]); } 
    size_t get_num_piecetype(PieceType pt) const
    { 
        Piece wp = piecetype_to_piece(pt, White);
        Piece bp = piecetype_to_piece(pt, Black);
        return get_num_piece(wp) + get_num_piece(bp);
    } 

    size_t get_total_piece_count(Colour us) const 
    {
        size_t count = 0;
        Piece lower = us == White ? WQ : BQ;
        Piece upper = us == White ? WQ : BP;
        for(Piece p = lower; p <= upper; ++p) {
            count += get_num_piece(p);
        }
        return count;
    }
    size_t get_total_piece_count() const 
    {
        return get_total_piece_count(White) + get_total_piece_count(Black);
    }

    size_t get_ply_count() const { return state.ply_count; }
    std::string get_algebraic(BMove m) const;
    Bitboard attacks_to(Square sq) const;
    bool is_repeat() const { return state.repitition; }

    bool attacked(Square sq, Colour by) const;
    bool in_check(Colour us) const;
    bool in_checkmate(Colour us) const;
    bool can_castle(Colour us, Move type) const;
    bool in_endgame() const { return state.endgame; }

    void print_squares() const;
    void print_move(BMove m) const;
    void print_moves() const;
    void print_occupied() const;
    void print_context(BMove m, bool capture, Move flag) const;

private: 

    State state; 
    State prev_state;
    std::vector<BMove> movelist;
    std::vector<ZobristKey> history;
    Bitboard piece_bbs[12];
    Piece squares[64]; // Square centric lookup 
    Bitboard occ;
    Bitboard white_occ;
    Bitboard black_occ;

    void init_squares(std::string fen);
    void init_bbs();

    void init_behind();
    void init_piece_attacks();
    void init_attacks();

    void castle_kingside();
    void uncastle_kingside();
    void castle_queenside();
    void uncastle_queenside();
    void do_castle(Square rook_from, Square rook_to, Square king_from, Square king_to);

    void move_piece(Square from, Square to);
    void move_piece(Square from, Square to, Piece p);
    void remove_piece(Square sq);
    void put_piece(Square sq, Piece p);

    void promote(PieceType pt, Square sq, Colour us);

    bool board_ok();
    // Attacks 
    inline Bitboard blockers_and_beyond(PieceType pt, Square from) const;
    inline Bitboard pawn_squares(Square origin, Colour us) const;
};

#endif
