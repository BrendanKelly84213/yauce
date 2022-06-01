#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>

typedef uint64_t Bitboard;
typedef uint16_t BMove;

enum Piece : int { 
    BQ, BK, BR, BN, BB, BP, 
    WQ, WK, WR, WN, WB, WP,None=-1
};

enum PieceType : int {
    Queen, King, Rook, Knight, Bishop, Pawn, Null=-1
};

enum Square : int {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2, 
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4, 
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8, NullSquare = -1
};

enum Colour { White, Black };

enum Direction : int { 
    N   =    8, S   =   -8, E   =    1,  W   =   -1, //Basic compass rose 
    NE  =  N+E, NW  =  N+W, SE  =  S+E,  SW  =  S+W, 
    NEE = NE+E, NNE = N+NE, NNW =  N+NW, NWW = NW+W, //Knight Compass rose		
    SWW = SW+W, SSW = S+SW, SSE =  S+SE, SEE = SE+E
}; 

enum Move : BMove {
    QUIET, 
    DOUBLE_PAWN_PUSH, 
    OO, 
    OOO, 
    EN_PASSANT, 
    PROMOTE_QUEEN, 
    PROMOTE_ROOK, 
    PROMOTE_KNIGHT, 
    PROMOTE_BISHOP 
};

constexpr Square operator+(Square a, Direction b) { return (Square)( (int)a + (int)b ); }
constexpr Colour operator!(Colour c) { return (Colour)( !( (bool)c ) ); }
constexpr void operator++(PieceType &pt) { pt = (PieceType)((int)pt + 1); }

#endif
