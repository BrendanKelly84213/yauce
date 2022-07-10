#include <SDL2/SDL_image.h>

#include "BoardPiece.h"
#include "../utils/bits.h"

void BoardPiece::init(Piece _p, Square _s, int w, Colour bottom_colour)
{
    p = _p;
    s = _s;
}

void BoardPiece::update_square(int x, int y, int w, Colour bottom_colour)
{
    int center_x = x + (0.5 * w);
    int center_y = y + (0.5 * w);
    size_t rank = y_to_rank(center_y, w, bottom_colour);
    size_t file = x_to_file(center_x, w, bottom_colour);
    s = square(rank, file);
}

void BoardPiece::promote(Piece _p)
{ 
    assert(piece_to_piecetype(p) == Pawn);
    p = _p;
}

bool BoardPiece::in_piece(int x, int y, int square_w, Colour bottom_colour) const 
{
    int px = square_to_x(s, square_w, bottom_colour);
    int py = square_to_y(s, square_w, bottom_colour); 
    return x > px && x < (px + square_w) && y > py && y < (py + square_w);  
}   
