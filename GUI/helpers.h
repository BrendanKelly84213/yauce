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

constexpr size_t x_to_file(int x, int square_w) { return floor(x / square_w); }
constexpr size_t y_to_rank(int y, int square_w) { return 7 - floor(y / square_w); }

constexpr Square square(size_t rank, size_t file)
{
    return (Square)(rank * 8 + file);
}

#endif