#include "Generator.h"

//TODO: 
//  Check for checks 
//  Sliding pieces
//  piece bitboards array

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

constexpr void set_bit(Bitboard &bb, int square) 
{ 
    bb |= 1ULL << square; 
}

inline Square pop_bit(Bitboard &bb)
{
	Square index = Square(__builtin_ctzll(bb));
    bb &= ~(1ULL << index);
	return index;
} 

Bitboard occ_squares(Piece* pieces, Colour colour)
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

Bitboard king_squares(
        Bitboard friend_occ, 
        Square origin)
{
    return king_mask(origin) & ~friend_occ;
}

Bitboard knight_squares(
        Bitboard friend_occ, 
        Square origin)
{
    return knight_mask(origin) & ~friend_occ;
}

Bitboard pawn_squares(
        Bitboard friend_occ, 
        Bitboard op_occ, 
        BoardState board_state, 
        Square origin)
{
    Colour us = board_state.side_to_move;
    Direction push_dir = us == White ? N : S;  
    Bitboard ratt = bit(origin + push_dir + E);
    Bitboard latt = bit(origin + push_dir + W);
    Bitboard single_push = bit(origin + push_dir);
    Bitboard double_push = bit(origin + push_dir + push_dir);
    Bitboard squares; 

    if(op_occ & ratt) {
        squares |= ratt;
    }
    
    if(op_occ & latt) {
        squares |= latt;
    }

    if(board_state.ep_file != -1) {
        int rank = floor(origin * 0.125);
        int file = origin % 8;
        if(us == White && rank == 4
        || (us == Black && rank == 3)) {
            squares |= ((board_state.ep_file < file) ? latt : ratt) ;
        } 
    }

    if((us == Black && origin >= a7 && origin <= h7) 
    || (us == White && origin >= a2 && origin <= h2)
    && !(double_push & op_occ)) {
        squares |= double_push;
    } else {
        if(!single_push & op_occ) {
            squares |= single_push;
        }
    }

    return squares & ~friend_occ;
}

#if 1
Move* generator(Bitboard * piece_bbs, 
        BoardState board_state, 
        Bitboard friend_occ, 
        Bitboard op_occ)
{   
    static Move moves[128];
    int i = 0;
    for(int p = Pawn; p < King; ++p) {
        Bitboard occ = piece_bbs[p];
        while(occ) {
            Square origin = pop_bit(piece_bbs[p]);
            moves[i].from = origin;
            Bitboard to_squares;
            switch(p) {
                case Pawn:
                    to_squares = pawn_squares(friend_occ, op_occ, board_state, origin);
                    break;
                case King: 
                    to_squares = king_squares(friend_occ, origin);
                    break;
                case Knight: 
                    to_squares = knight_squares(friend_occ, origin);
                    break;
                default: break;
            }
            while(to_squares) {
                moves[i].to = pop_bit(to_squares);
            }
            ++i;
        }
    }
    return moves;
}
#endif 

int main()
{
    BoardState board_state;
    std::string fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
    board_state.init(fen);
    return 0;
}
