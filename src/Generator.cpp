#include "Generator.h"

//TODO: 
//  Check for checks 
//  Sliding pieces
//  Organize code 


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

// Sliding piece lookup tables 
const Bitboard maindia = 0x8040201008040201ULL;
const Bitboard nort = 0x0101010101010100;
const Bitboard noea = 0x8040201008040200;
const Bitboard sout = 0x0080808080808080;

// For debugging
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

#if 0
static Bitboard diag_attacks[64];
static Bitboard anti_diag_attacks[64];
static Bitboard rank_attacks[64];
static Bitboard file_attacks[64];
#endif

static Bitboard queen_attacks[64];
static Bitboard rook_attacks[64];
static Bitboard bishop_attacks[64];

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

constexpr Square orig_bm(BMove m)  
{
    return static_cast<Square>((m >> 6) & 0x3f);
}

constexpr Square dest_bm(BMove m) 
{
    return static_cast<Square>(m & 0x3f);
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
            if(squares[s] >= 6 && squares[s] <= 11) {
                set_bit(occ, s);
            }
        } else {
            if(squares[s] >= 0 && squares[s] <= 5) {
                set_bit(occ, s);
            }
        }
    } 
    return occ;
}

// Naive sliding piece lookup table initialization
void init_sliding()
{
    //NOTE: Following to loops could be joined into one 
    Bitboard west = Rank1BB ^ 0x080ULL;
    Bitboard west_attacks[64];
    for(int r8=0; r8 < 64; r8+=8) {
        for(int f=0; f<8; ++f) {
            Square sq = static_cast<Square>(r8 + (7-f));
            west_attacks[sq] = ((west << r8) >> f) & (Rank1BB << r8);
        }
    }

    Bitboard east = Rank8BB ^ 0x0100000000000000ULL;
    Bitboard east_attacks[64];
    for(int mr8=0; mr8 < 64; mr8+=8) {
        int r8 = 63 - mr8;
        for(int f=0; f<8; ++f) {
            Square sq = static_cast<Square>(r8 - (7-f));
            east_attacks[sq] = ((east >> mr8) << f) & (Rank8BB >> mr8);
            /* print(east_attacks[sq]); */
        }
    }

    //NOTE: Following to loops could be joined into one 
    Bitboard north = FileABB ^ 0x01ULL; 
    Bitboard north_attacks[64]; 
    for(int s=0; s < 64; ++s) {
        north_attacks[s] = north << s;
    }

    Bitboard south = FileHBB ^ 0x8000000000000000ULL;
    Bitboard south_attacks[64];
    for(int i=0; i < 64; ++i) {
        Square sq = static_cast<Square>(63-i);
        south_attacks[sq] = south >> i;
    }

    //NOTE: Following to loops could be joined into one 
    Bitboard northeast = 0x8040201008040200ULL;
    Bitboard ne_attacks[64];
    for(int s=0; s<64; ++s) {
        int file = s % 8;  
        Bitboard file_mask = 0;
        for(int f=file; f < 8; ++f) {
            file_mask |= (FileABB << f);
        }
        ne_attacks[s] = (northeast << s) & file_mask;
    }
    
    Bitboard southwest = 0x8040201008040201ULL ^ 0x8000000000000000ULL;
    Bitboard sw_attacks[64];
    for(int s=0; s<64; ++s) {
        Square sq = static_cast<Square>(63-s);
        int file = sq % 8;  
        Bitboard file_mask = 0;
        for(int f=file; f >=0 ; --f) {
            file_mask |= (FileABB << f);
        }
        sw_attacks[sq] = (southwest >> s) & file_mask;
    }

    // NOTE: Same loop as west attacks
    Bitboard northwest = 0x0102040810204080ULL ^ 0x080ULL;
    Bitboard nw_attacks[64];
    for(int r8=0; r8 < 64; r8+=8) {
        for(int f=0; f<8; ++f) {
            int file = 7-f;
            Bitboard file_mask = 0;
            for(int f=file; f >=0 ; --f) {
                file_mask |= (FileABB << f);
            }

            Square sq = static_cast<Square>(r8 + file);
            nw_attacks[sq] = ((northwest << r8) >> f) & file_mask;
        }
    }

    // NOTE: Same loop as east attacks
    Bitboard southeast = 0x0102040810204080ULL ^ 0x0100000000000000ULL;
    Bitboard se_attacks[64];
    for(int mr8=0; mr8<64; mr8+=8) {
        int r8 = 63 - mr8;
        for(int f=0; f<8; ++f) {
            Bitboard file_mask = 0;
            for(int file = f; file < 8; ++file) {
                file_mask |= (FileABB << file);
            }

            Square sq = static_cast<Square>(r8 - (7-f));
            se_attacks[sq] = ((southeast << f) >> mr8) & file_mask;
        }
    }

    for(int s = 0; s < 64; ++s) {
        rook_attacks[s] = north_attacks[s] | south_attacks[s] | west_attacks[s] | east_attacks[s];
        bishop_attacks[s] = nw_attacks[s] | se_attacks[s] | ne_attacks[s] | sw_attacks[s];
        queen_attacks[s] = rook_attacks[s] | bishop_attacks[s];
    }
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

Bitboard king_squares(
        Bitboard friend_occ, 
        Square origin)
{
    return king_mask(origin) & ~friend_occ;
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

    // Attacks 
    if(op_occ & ratt) {
        squares |= ratt;
    }
    if(op_occ & latt) {
        squares |= latt;
    }

    //En Passant
    if(board_state.ep_file != -1) {
        int rank = floor(origin * 0.125);
        int file = origin % 8;
        if(us == White && rank == 4
        || (us == Black && rank == 3)) {
            squares |= ((board_state.ep_file < file) ? latt : ratt) ;
        } 
    }

    // Double push 
    if((us == Black && origin >= a7 && origin <= h7) 
    || (us == White && origin >= a2 && origin <= h2)) {
        if(!(double_push & op_occ)) {
            squares |= double_push;
        }
    }

    if(!(single_push & op_occ)) {
        squares |= single_push;
    }

    return squares & mask & ~friend_occ;
}


BMove* generator(Bitboard * piece_bbs, 
        BoardState board_state, 
        Bitboard friend_occ, 
        Bitboard op_occ)
{   
    static BMove moves[128];
    int i = 0;
    for(int p = Pawn; p <= King; ++p) {
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

            BMove from = static_cast<BMove>((origin << 6) & 0xfc0);
            while(to_squares) {
                Square dest = pop_bit(to_squares);
                BMove to = static_cast<BMove>(dest & 0x3f);
                moves[i] = from | to;
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

// For GUI 
std::vector<Move> boardstate_to_move_vec(BoardState board_state)
{
    std::vector<Move> move_vec;

    Bitboard piece_bbs[6];
    Colour us = board_state.side_to_move;
    Bitboard friend_occ = occ_squares(board_state.squares, us);
    Bitboard op_occ = occ_squares(board_state.squares, (us == White ? Black : White));
    BMove * moves;

    for(int p = Pawn; p <= King; ++p) {
        piece_bbs[p] = 0;
    }

    for(Square s = a1; s <= h8; ++s) {
        Piece piece = board_state.squares[s];
        PieceType pt = piece_to_piecetype(piece); 
        if(pt != Null) {
            if((us == White && piece >= 6 && piece <= 11)
            || (us == Black && piece >= 0 && piece <= 5)) {
                piece_bbs[pt] |= bit(s);
            }
        }
    }

    moves = generator(piece_bbs, board_state, friend_occ, op_occ);
    while(*moves != 0x00) {
        BMove bmove = *moves++;
        Move move;
        move.from = orig_bm(bmove);
        move.to = dest_bm(bmove);
            move_vec.push_back(move);
    }
    return move_vec;
}

#if 1
int main()
{
    BoardState board_state;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    init_sliding();
    for(int s=a1; s<=h8; ++s) {
    }

    return 0;
}
#endif
