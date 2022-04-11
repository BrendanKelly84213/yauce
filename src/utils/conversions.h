#ifndef CONVERSION_H
#define CONVERSION_H

#include <string>
#include "types.h"

std::string square_to_str(int idx);
std::string piece_to_str(int idx);
std::string piecetype_to_str(int idx);
PieceType piece_to_piecetype(Piece p);
std::string flag_to_str(int idx);

#endif
