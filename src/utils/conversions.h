#ifndef CONVERSION_H
#define CONVERSION_H

#include <string>
#include "types.h"

namespace conversions {
std::string square_to_str(int idx);
std::string piece_to_str(int idx);
std::string piecetype_to_str(int idx);
int piece_to_piecetype(int idx);
std::string flag_to_str(int idx);
};

#endif
