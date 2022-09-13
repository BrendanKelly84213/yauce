#include "bits.h"
#include <math.h>

Bitboard trace_ray(Square origin, Direction d)
{
    Bitboard ray = 0ULL;
    for(Square s=origin + d; in_bounds(s,d); s = s + d) {
        ray |= bit(s);  
    }
    return ray;
}

// For debugging
void print(Bitboard bb) 
{
    for(int y=7; y >=0; --y){
        std::cout << '\n';
        for(int x=0; x < 8; ++x)
            std::cout << get_bit(bb, x,y) << " ";
    }
    std::cout << '\n';
}

void print16(BMove m)
{
    for(size_t i=0; i < 16; ++i)
        std::cout << ( (m >> i) & 1ULL );
    std::cout << '\n';
}
