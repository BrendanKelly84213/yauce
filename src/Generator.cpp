#include "Generator.h"

//TODO: 
//  Check for checks 
//  Sliding pieces
//  Organize code 

typedef uint16_t BMove;

// BMove:
// Bits 0 - 5: from 
// Bits 6 - 11: to
// Bits 11 - 16: empty for now

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

Square& operator++(Square& s) 
{ 
    s = static_cast<Square>(int(s) + 1); 
    return s;
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

Bitboard occ_squares(Piece* squares, Colour colour)
{
    Bitboard occ = 0x00;
    for(int s=0; s<64; ++s) {
        if(colour == White) {
            if(squares[s] >= WQ && squares[s] <= WP) {
                set_bit(occ, s);
            }
        } else {
            if(squares[s] >= BQ && squares[s] <= BP) {
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

    Bitboard mask = ratt | latt | single_push | double_push;
    Bitboard squares = 0; 

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
    || (us == White && origin >= a2 && origin <= h2)) {
        if(!(double_push & op_occ)) {
            squares |= double_push;
        }
        if(!(single_push & op_occ)) {
            squares |= single_push;
        }
    }

    return squares & mask & ~friend_occ;
}


std::string square_to_str[64] = {  
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", 
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", 
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", 
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", 
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", 
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", 
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", 
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8" 
};

constexpr Square orig_bm(BMove m)  
{
    return static_cast<Square>((m >> 6) & 0x3f);
}

constexpr Square dest_bm(BMove m) 
{
    return static_cast<Square>(m & 0x3f);
}

BMove* generator(Bitboard * piece_bbs, 
        BoardState board_state, 
        Bitboard friend_occ, 
        Bitboard op_occ)
{   
    static BMove moves[128];
    int i = 0;
    for(int p = Pawn; p < King; ++p) {
        Bitboard occ = piece_bbs[p];
        while(occ) {
            Square origin = pop_bit(occ);
            moves[i] = 0;
            Bitboard to_squares = 0;
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
                Square dest = pop_bit(to_squares);
                moves[i] |= static_cast<BMove>((origin << 6) & 0xfc0);
                moves[i] |= static_cast<BMove>(dest & 0x3f);
                ++i;
            }
        }
    }
    return moves;
}

PieceType piece_to_piecetype(Piece piece) 
{
    switch(piece) {
        case WP:
        case BP:
            return Pawn;
        case WN:
        case BN:
            return Knight;
        case WB:
        case BB:
            return Bishop;
        case WR:
        case BR:
            return Rook;
        case WQ:
        case BQ:
            return Queen;
        case WK:
        case BK:
            return King;
        default: break;
    }
    return Null;
}

int main()
{
    BoardState board_state;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    board_state.init(fen);
    Bitboard piece_bbs[6];
    Colour us = board_state.side_to_move;
    Bitboard friend_occ = occ_squares(board_state.squares, us);
    Bitboard op_occ = occ_squares(board_state.squares, (us == White ? Black : White));

    for(Square s = a1; s <= h8; ++s) {
        Piece piece = board_state.squares[s];
        PieceType pt = piece_to_piecetype(piece); 
        if(pt != Null) {
            if((us == White && piece >= WQ && piece <= WP)
            || (us == Black && piece >= BQ && piece <= BP))
               piece_bbs[pt] |= bit(s);
        }
    }

    BMove * moves;
    moves = generator(piece_bbs, board_state, friend_occ, op_occ);

    return 0;
}
