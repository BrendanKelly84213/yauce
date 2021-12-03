#include "Generator.h"

//TODO: 
//  Check for checks 
//  Castling
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
static Bitboard queen_attacks[64];
static Bitboard rook_attacks[64];
static Bitboard bishop_attacks[64];
static Bitboard behind[64][64];
static Bitboard piece_attacks[6][64];
const Direction directions[] = { N, S, E, W, NE, NW, SE, SW };

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

void print(Bitboard bb) 
{
    for(int y=7; y >=0; --y){
        std::cout << '\n';
        for(int x=0; x < 8; ++x)
            std::cout << get_bit(bb, x,y) << " ";
    }
    std::cout << '\n';
}

constexpr Bitboard bit(int s)
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

Bitboard get_behind(Square from, Square to)
{
    return behind[from][to];
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
        if(colour == White && squares[s] >= 6 && squares[s] <= 11
        || colour == White && squares[s] >= 0 && squares[s] <= 5) {
                set_bit(occ, s);
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

int distance(int origin, int dest)
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

Bitboard trace_ray(int origin, Direction d)
{
    Bitboard ray = 0ULL;
    for(int s=origin + d; in_bounds(s,d); s += d) {
        ray |= bit(s);  
    }
    return ray;
}

void init_behind()
{
    for(Square f=a1; f<=h8; ++f) {
        for(int di=0; di<8; ++di) { 
            Direction d = directions[di];
            for(int t=f+d; in_bounds(t, d); t+=d) {
                behind[f][t] = trace_ray(t,d);
            }
        }
    }
}

void init_piece_attacks()
{
    for(Square sq=a1; sq<=h8; ++sq) {
        Bitboard ne = trace_ray(sq, NE);
        Bitboard sw = trace_ray(sq, SW);
        Bitboard nw = trace_ray(sq, NW);
        Bitboard se = trace_ray(sq, SE);

        Bitboard n = trace_ray(sq, N);
        Bitboard s = trace_ray(sq, S);
        Bitboard e = trace_ray(sq, E);
        Bitboard w = trace_ray(sq, W);

        piece_attacks[Bishop][sq] = ne | sw | nw | se; 
        piece_attacks[Rook][sq] = n | w | e | s; 
        piece_attacks[Queen][sq] = piece_attacks[Bishop][sq] | piece_attacks[Rook][sq];

        piece_attacks[Knight][sq] = knight_mask(sq);
        piece_attacks[King][sq] = king_mask(sq);
    }
}

void init_generator() 
{
    init_behind();
    init_piece_attacks();
}

Bitboard get_piece_moves(PieceType p, Square from, Bitboard occ)
{
    Bitboard ts = piece_attacks[p][from];
    Bitboard edges = FileABB | FileHBB | Rank1BB | Rank8BB; 
    Bitboard bb = p == Knight || p == King ? occ : occ & ~edges;
    while(bb) {
        Square to = pop_bit(bb);
        ts &= ~behind[from][to];
    }
    return ts;
}

Bitboard knight_squares(
        Bitboard friend_occ, 
        Square origin)
{
    return knight_mask(origin) & ~friend_occ;
}

Bitboard king_squares(
        Bitboard friend_occ, 
        Square origin)
{
    return king_mask(origin) & ~friend_occ;
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
            if(p == Pawn) {
                to_squares = pawn_squares(friend_occ, op_occ, board_state, origin);
            } else {
                PieceType pt = static_cast<PieceType>(p);
                to_squares = get_piece_moves(pt, origin, friend_occ | op_occ) & ~friend_occ; 
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

#if 0
int main()
{
    BoardState board_state;
    std::string fen = "8/4kP1r/n4p2/2P1N1b1/4KQ1R/P6q/R3p3/2r5 w - - 0 1";
    board_state.init(fen);
    Bitboard occ = occ_squares(board_state.squares, Black) ;

    init_behind();
    init_piece_attacks();

    print(behind[e1][e2]);
    /* print(trace_ray(e2, N)); */

    /* for(Square f=a1; f<h8; ++f) { */
    /*     for(Square t=a1; t<h8; ++t) { */
    /*         if(behind[f][t]) { */
    /*             std::cout << "Origin and blocker: " << '\n'; */
    /*             print(bit(f) | bit(t)); */

    /*             std::cout << "'\n'behind: " << '\n'; */
    /*             print(behind[f][t]); */
    /*         } */
    /*     } */
    /* } */


    return 0;
}
#endif
