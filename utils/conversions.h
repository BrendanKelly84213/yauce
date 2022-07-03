#ifndef CONVERSION_H
#define CONVERSION_H

#include <string>
#include "types.h"

Square string_to_square(std::string str);
std::string square_to_str(int idx);
std::string piece_to_str(int idx);
std::string piecetype_to_str(int idx);
std::string piecetype_to_algstr(int idx);
PieceType piece_to_piecetype(Piece p);
Piece piecetype_to_piece(PieceType pt, Colour us);
std::string flag_to_str(int idx);
std::string promote_flag_to_str(Move flag);
std::string colour_to_str(Colour c);
PieceType promotion_to_piecetype(Move flag);
Colour piece_to_colour(Piece p);


#endif
