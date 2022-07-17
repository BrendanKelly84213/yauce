#include "zobrist.h"

std::mt19937 mt(01234567);

Bitboard random_num()
{ 
    std::uniform_int_distribution<Bitboard> dist(0, UINT64_MAX); 
    return dist(mt); 
}

void Zobrist::init_table()
{
    for(Piece p = BQ; p <= WP; ++p) {
        for(Square s = a1; s <= h8; ++s) {
            table[p][s] = random_num();
        }
    }
}

Bitboard Zobrist::compute_hash()
{
    Bitboard h = 0;
    for(Piece p = BQ; p <= WP; ++p) {
        for(Square s = a1; s <= h8; ++s) {
            h ^= table[p][s];
        }
    }
    return h;
}
