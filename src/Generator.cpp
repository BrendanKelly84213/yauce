#include "Generator.h"

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

template<typename T>
constexpr Square operator+(Square a, T b) 
{ 
    return static_cast<Square>(int(a) + int(b)); 
}

constexpr Square operator++(Square s) 
{ 
    return static_cast<Square>(int(s) + int(s)); 
}

constexpr Bitboard get_bit(Bitboard bb, int square) 
{ 
    return (bb >> square & 1ULL);
}

constexpr Bitboard get_bit(Bitboard bb, int x, int y) 
{ 
    return (bb >> y*8 + x) & 1ULL;
}

void print(Bitboard bb) {
	for(int y=7; y >=0; --y){
		std::cout << '\n';
		for(int x=0; x < 8; ++x)
			std::cout << get_bit(bb, x,y) << " ";
	}
	std::cout << '\n';
}

constexpr Bitboard bit(Square s)
{
    return 1ULL << s;
}

Bitboard king_mask(Square origin)
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

Bitboard king_squares(
        Bitboard friend_occ, 
        BoardState board_state, 
        Square origin)
{
    //TODO: Check for checks 
    return king_mask(origin) & ~friend_occ;
}

constexpr void set_bit(Bitboard &bb, int square) 
{ 
    bb |= 1ULL << square; 
}

Bitboard friendly_occ_squares(Piece* pieces, Colour colour)
{
    Bitboard occ = 0x00;
    for(int s=0; s<64; ++s) {
        if(colour == White) {
            if(pieces[s] >= WQ && pieces[s] <= WP) {
                set_bit(occ, s);
            }
        } else {
            if(pieces[s] >= BQ && pieces[s] <= BP) {
                set_bit(occ, s);
            }
        }
    } 
    return occ;
}

int main()
{
    BoardState board_state;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    board_state.init(fen);

    Bitboard friend_occ = friendly_occ_squares(board_state.squares, Black);

    print(friend_occ);
    std::cout << board_state.squares[56] << '\n';
    print(king_squares(friend_occ, board_state, a1));
    return 0;
}
