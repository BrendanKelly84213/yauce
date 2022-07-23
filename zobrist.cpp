#include "zobrist.h"

ZobristKey table[12][64];
ZobristKey black_to_move;

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
    black_to_move = random_num();
}

// Initial position
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

// If the given hash is white to move, then the new hash will be black to move and vice versa
Bitboard new_hash_colour(Bitboard hash)
{
    return hash ^ black_to_move;
}
