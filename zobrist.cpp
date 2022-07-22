#include "zobrist.h"

Bitboard table[12][64];
std::mt19937 mt(01234567);

Bitboard random_num()
{ 
    std::uniform_int_distribution<Bitboard> dist(0, UINT64_MAX); 
    return dist(mt); 
}

void init_table()
{
    for(Piece p = BQ; p <= WP; ++p) {
        for(Square s = a1; s <= h8; ++s) {
            table[p][s] = random_num();
        }
    }
}

Bitboard compute_hash()
{
    Bitboard h = 0;
    for(Piece p = BQ; p <= WP; ++p) {
        for(Square s = a1; s <= h8; ++s) {
            h ^= table[p][s];
        }
    }
    return h;
}

Bitboard updated_hash(Bitboard hash, Piece p, Square s)
{
    return hash ^ table[p][s];
}
