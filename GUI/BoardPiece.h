#ifndef PIECE_H
#define PIECE_H

#include <string>
#include <cassert>
#include "../utils/types.h"
#include "../utils/conversions.h"


struct BoardPiece {

    bool dragging;
    Piece p;
    Square s;

    BoardPiece() : dragging(false), p(None), s(NullSquare)
    {
    }

    void init(Piece _p, Square _s, int w, Colour pc);
    void update_square(int x, int y, int w, Colour pc);
    void promote(Piece _p, SDL_Renderer* renderer);

    bool in_piece(int x, int y, int square_w, Colour bottom_colour) const; 
};

#endif
