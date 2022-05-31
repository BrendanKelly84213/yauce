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


// For Pretty Printing and debugging mostly
struct MoveInfo {
    int score;
    BMove m;
    PieceType moved;
    PieceType captured;
    Colour side;
    std::string algebraic;

    MoveInfo(const MoveInfo & mi) 
        : score(mi.score), m(mi.m), moved(mi.moved), captured(mi.captured), side(mi.side), algebraic(mi.algebraic) 
    {
    }

    /* MoveInfo(const MoveInfo & mi) */
    /* { */
    /*     score = mi.score; */
    /*     m = mi.m; */
    /*     moved = mi.moved; */
    /*     captured = mi.captured; */
    /*     side = mi.side; */
    /*     algebraic = mi.algebraic; */
    /* } */

    MoveInfo() 
        : score(0), m(0), moved(Null), captured(Null), side(White), algebraic("") 
    {
    }

    MoveInfo(BMove _m, PieceType _moved, PieceType _captured, Colour _side, bool check = false, int _score = 0) 
        : m(_m), moved(_moved), captured(_captured), side(_side), score(_score) 
    {
        // TODO: checks, other flags, etc...
        std::string tosq = square_to_str(get_to(m));
        Move flag = get_flag(m);
        if(flag == OO || flag == OOO) {
            algebraic = flag_to_str(flag);
        } else if(_moved == Pawn) {
            if(captured == Null) 
                algebraic = tosq;
            else algebraic = "Px" + tosq;
            if(flag >= PROMOTE_QUEEN && flag <= PROMOTE_BISHOP)
                algebraic += promote_flag_to_str(flag);
        } else {
            algebraic = piecetype_to_algstr(_moved) + (captured != None ? "x" : "") + tosq;
        }

        if(check) 
            algebraic += "+";
    }

};

// Movelist wrapper class for ongoing game movelist
// Not to be confused with our generated moves array, which is a raw C style array 
// containing unsigned 16 bit encoded moves 
class MoveList {
private: 
    std::vector<MoveInfo> movelist;
public: 

    MoveList() {}

    MoveList(size_t size)
    {
        movelist.reserve(size);
    }

    void add(BMove m, 
            PieceType moved, 
            PieceType captured, 
            Colour side,
            bool check = false,
            int score = 0) 
    { 
        const MoveInfo mi(m, moved, captured, side, check, score);
        movelist.push_back(mi); 
    }

    void insert(size_t i, MoveInfo mi) { movelist[i] = mi; };
    void remove() { movelist.pop_back(); }
    void print_moves() const;
    void print_moves_and_scores() const;

    void sort_by_score(Colour us);

    MoveInfo get_latest() const { return movelist.back(); }
};

struct State {
    Colour side_to_move = White; 
    // TODO: Castle can be a lot more lightweight...
    bool w_castle_ks = false; 
    bool w_castle_qs = false; 
    bool b_castle_ks = false; 
    bool b_castle_qs = false; 
    Square ep_square = NullSquare;
    // FIXME: int -> size_t
    int halfmove_clock = 0;
    int ply_count = 0;     
    Piece last_captured = None;

    bool operator==(State &b) const;
};

class BoardState {
private: 
    State state; 
    State prev_state;
    MoveList movelist;
    Bitboard piece_bbs[12];
    Piece squares[64]; // Square centric lookup 
    Bitboard occ = 0ULL;
    Bitboard white_occ = 0ULL;
    Bitboard black_occ = 0ULL;

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

public: 

    void init(std::string fen);

    void make_move(BMove m);
    void unmake_move(BMove m);

    Bitboard get_occ() const; 
    Bitboard get_friend_occ() const;
    Bitboard get_friend_occ(Colour us) const;
    Bitboard get_op_occ() const;
    Bitboard get_op_occ(Colour us) const;
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
    MoveList get_movelist() const { return movelist; }
    int get_opposite_end(Colour us) const { return us == White ? 7 : 0; }
    size_t get_num_piece(Piece p) const { return popcount(piece_bbs[p]); } 

    Bitboard attacks_to(Square sq) const;
    bool attacked(Square sq, Colour by) const;
    bool in_check(Colour us) const;
    bool in_checkmate(Colour us) const;
    bool can_castle(Colour us, Move type) const;

    void print_squares() const;
    void print_move(BMove m) const;
    void print_moves() const { movelist.print_moves(); }
    void print_occupied() const;
    void print_context(BMove m, bool capture, Move flag) const;

    bool operator==(BoardState b) const;
};

#endif
