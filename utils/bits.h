#include <cstdint>
#include <iostream>
#include "types.h"
#include "conversions.h"

//Ranks and files. 
constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileBBB = FileABB << 1;
constexpr Bitboard FileCBB = FileABB << 2;
constexpr Bitboard FileDBB = FileABB << 3;
constexpr Bitboard FileEBB = FileABB << 4;
constexpr Bitboard FileFBB = FileABB << 5;
constexpr Bitboard FileGBB = FileABB << 6;
constexpr Bitboard FileHBB = FileABB << 7;

constexpr Bitboard Rank1BB = 0xFF;
constexpr Bitboard Rank2BB = Rank1BB << (8 * 1);
constexpr Bitboard Rank3BB = Rank1BB << (8 * 2);
constexpr Bitboard Rank4BB = Rank1BB << (8 * 3);
constexpr Bitboard Rank5BB = Rank1BB << (8 * 4);
constexpr Bitboard Rank6BB = Rank1BB << (8 * 5);
constexpr Bitboard Rank7BB = Rank1BB << (8 * 6);
constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);


const Direction directions[] = { N, S, E, W, NE, NW, SE, SW };

void print(Bitboard bb);
void print16(BMove m);
Bitboard trace_ray(Square origin, Direction d);

constexpr Bitboard get_bit(Bitboard bb, int square) 
{ 
    return (bb >> square & 1ULL);
}

constexpr Bitboard get_bit(Bitboard bb, int x, int y) 
{ 
    return (bb >> y*8 + x) & 1ULL;
}

constexpr Bitboard bit(Square s)
{
    return 1ULL << s;
}

constexpr void set_bit(Bitboard &bb, Square square) 
{ 
    bb |= 1ULL << square; 
}

constexpr void clear_bit(Bitboard &bb, Square square)
{
    bb &= ~(1ULL << square);
}

constexpr BMove move(Square from, Square to, Move flag)
{
    return static_cast<BMove>(((from << 10) | (to << 4)) | flag);
}

constexpr Square get_to(BMove m)  
{
    return static_cast<Square>((m >> 4) & 0x3f);
}

constexpr Square get_from(BMove m) 
{
    return static_cast<Square>((m >> 10) & 0x3f);
}

constexpr Move get_flag(BMove m) 
{
    return static_cast<Move>(m & 0xf);
}

inline Square pop_bit(Bitboard &bb)
{
    Square index = Square(__builtin_ctzll(bb));
    bb &= ~(1ULL << index);
    return index;
} 

constexpr Square msb(Bitboard bb)
{
    return static_cast<Square>(__builtin_clzll(bb));
}

constexpr Square lsb(Bitboard bb)
{
    return static_cast<Square>(__builtin_ctzll(bb));
}

constexpr size_t popcount(Bitboard bb)
{
    return __builtin_popcountll(bb);
}

// FIXME: return unsigned, take Square, Square 
constexpr int distance(int origin, int dest)
{
    int r2 = dest >> 3;
    int r1 = origin >> 3;
    int f2 = dest % 8;
    int f1 = origin % 8; 
    int r1r2 = std::abs(r2 - r1); 
    int f1f2 = std::abs(f2 - f1);

    return std::max(f1f2, r1r2);
}

constexpr bool in_bounds(int s, Direction d)
{
     return s >= a1 && s <= h8 && distance(s, s-d) == 1;
}

constexpr Bitboard king_mask(Square origin)
{
    return (bit(origin + N) & ~Rank1BB)
        | (bit(origin + NE) & ~(FileABB | Rank1BB))
        | (bit(origin + E) & ~FileABB)
        | (bit(origin + SE) & ~(FileABB | Rank8BB))
        | (bit(origin + S) & ~Rank8BB)
        | (bit(origin + SW) & ~(FileHBB | Rank8BB))
        | (bit(origin + W) & ~FileHBB)
        | (bit(origin + NW) & ~(FileHBB | Rank1BB));
}

constexpr Bitboard knight_mask(Square origin)
{
    return (bit(origin + NEE) & ~(FileABB | FileBBB | Rank1BB))
        | (bit(origin + NNE) & ~(FileABB | Rank1BB | Rank2BB))
        | (bit(origin + NNW) & ~(FileHBB | Rank1BB | Rank2BB))
        | (bit(origin + NWW) & ~(FileHBB | FileGBB | Rank1BB))
        | (bit(origin + SWW) & ~(FileHBB | FileGBB | Rank8BB))
        | (bit(origin + SSW) & ~(FileHBB | Rank8BB | Rank7BB))
        | (bit(origin + SSE) & ~(FileABB | Rank8BB | Rank7BB))
        | (bit(origin + SEE) & ~(FileABB | FileBBB | Rank8BB));
}

constexpr Bitboard pawn_captures_mask(Square origin, Colour us)
{
    Direction push_dir = us == White ? N : S;  

    Bitboard ratt = bit(origin + push_dir + E) & ~FileABB;
    Bitboard latt = bit(origin + push_dir + W) & ~FileHBB;

    return ratt | latt;
}

constexpr Bitboard pawn_mask(Square origin, Colour us)
{
    Direction push_dir = us == White ? N : S;  
    Bitboard single_push = bit(origin + push_dir);

    return pawn_captures_mask(origin, us) | single_push;
}   

constexpr Square square(int rank, int file)
{
    return (Square)(rank * 8 + file);
}

constexpr int rank(Square s)
{
    return s >> 3;
}

constexpr int file(Square s)
{
    return s % 8;
}

// NOTE: These functions are only relevant to the GUI,
// Could not figure out linker errors, so I'm copping out

inline int square_to_x(Square s, int square_w) 
{
    size_t f = file(s);
    return f * square_w;
}

inline int square_to_y(Square s, int square_w) 
{
    size_t r = rank(s);
    return (7 - r) * square_w;
}

inline size_t x_to_file(int x, int square_w) { return floor(x / square_w); }
inline size_t y_to_rank(int y, int square_w) { return 7 - floor(y / square_w); }

inline Square xy_to_square(int x, int y, int square_w)
{
    size_t rank = y_to_rank(y, square_w);
    size_t file = x_to_file(x, square_w);
    return square(rank, file);
}

