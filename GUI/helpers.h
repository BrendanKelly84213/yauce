#ifndef HELPERS_h
#define HELPERS_h

#include "../utils/types.h"

constexpr size_t rank(Square s) { return s >> 3; }
constexpr size_t file(Square s) { return s % 8; }

// Top left hand corner of square in 
constexpr int square_to_x(Square s, int square_w) 
{
    size_t f = file(s);
    return f * square_w;
}

constexpr int square_to_y(Square s, int square_w) 
{
    size_t r = rank(s);
    return (7 - r) * square_w;
}

#endif