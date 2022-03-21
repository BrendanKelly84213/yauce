#include "BoardState.h"

enum Section 
{ 
    Board, 
    SideToMove, 
    CastlingRights, 
    EPSquare, 
    HalfmoveClock, 
    FullmoveCounter 
};


bool is_piece_ch(char ch);
Piece fen_to_piece(char ch);

constexpr Bitboard temp_get_bit(Bitboard bb, int x, int y) 
{ 
    return (bb >> y*8 + x) & 1ULL;
}

void temp_print(Bitboard bb) 
{
    for(int y=7; y >=0; --y){
        std::cout << '\n';
        for(int x=0; x < 8; ++x)
            std::cout << temp_get_bit(bb, x,y) << " ";
    }
    std::cout << '\n';
}

void print_squares(Piece squares[64])
{
    for(int s = a1; s <= h8; ++s) {
        if(!(s % 8)) {
            if(s == 0)
                std::cout << "\n";
            else 
                std::cout << "|\n";
        }

        if(squares[s] == -1) {
            std::cout << "|##";
        } else {
            std::cout << "|" << conversions::piece_to_str(squares[s]);
        }
        if(s == h8)
            std::cout << "|";
    }
    std::cout << "\n";
}

// TODO: 

void BoardState::init_squares(std::string fen)
{
    // Parse out pieces 
    int rank=7;
    int file=0;
    int i=0;
    int section=0;
    
    for(int i=a1; i<=h8; ++i) {
        squares[i] = None;
    }

    while(fen[i] != ' ') {
        int sq = (rank)*8 + file;

        if(fen[i] == '/') {
            rank--;
            file=0;
        }

        if(fen[i] >= '0' && fen[i] <= '8') {
            file += (fen[i] - 0x30);
        }

        if(is_piece_ch(fen[i])) {
            squares[sq] = fen_to_piece(fen[i]);
            file++;
        } 

        i++;
    }

    section++;

    // Parse out info
    std::string info = fen.substr(i + 1, fen.length());
    for(int i=0; i<info.length(); ++i) {
        if(info[i] == ' ') {
            section++;
            continue;
        }
        switch(section) {
            case SideToMove: 
                if(info[i] == 'w') {
                    state.side_to_move = White;
                } else {
                    state.side_to_move = Black;
                }
                break;
            case CastlingRights:
                switch(info[i]) {
                    case '-':
                        state.w_castle_ks = false; 
                        state.w_castle_qs = false; 
                        state.b_castle_ks = false; 
                        state.b_castle_qs = false; 
                        break;
                    case 'k':
                        state.b_castle_ks = true;
                        break;
                    case 'q':
                        state.b_castle_qs = true;
                        break;
                    case 'K': 
                        state.w_castle_ks = true;
                        break;
                    case 'Q':
                        state.w_castle_qs = true;
                        break;
                    default: break;
                }
                break;
            case EPSquare:
                if(info[i] == '-') {
                    state.ep_file = -1;
                } else if(info[i] >= 'a' && info[i] <= 'h') {
                    state.ep_file = static_cast<int>(info[i] - 0x61);
                } 
                break;
            case HalfmoveClock:
                state.halfmove_clock = static_cast<int>(info[i] - 0x30);
                break;
            case FullmoveCounter:
                state.ply_count = static_cast<int>(info[i] - 0x30);
                break;
            default: break;
        }
    }
}

void BoardState::init_bbs() 
{
    white_occ = 0ULL; 
    black_occ = 0ULL;
    occ = 0ULL;
    for(int p=BQ; p<=WP; ++p) {
        piece_bbs[p] = 0ULL;
    }

    for(int s = 0; s<64; ++s) {
        int p = (int)squares[s];
        if(p >= 0) { 
            int pt = conversions::piece_to_piecetype(p);
            piece_bbs[p] |= (1ULL << s);
            occ |= piece_bbs[p];
            if(p >= WQ && p <= WP) {
                white_occ |= piece_bbs[p];
            } else {
                black_occ |= piece_bbs[p];
            }
        }
    }
}

void BoardState::init(std::string fen)
{
    init_squares(fen);
    init_bbs(); 
}

void BoardState::do_castle(int rook_from, int rook_to, int king_from, int king_to)
{
    Piece rook = state.side_to_move == White ? WR : BR;
    Piece king = state.side_to_move == White ? WK : BK;

    // Squares
    squares[king_from] = None;
    squares[rook_from] = None;
    squares[king_to] = king;
    squares[rook_to] = rook;
    // Bitboards
    piece_bbs[king] &= ~(1ULL << king_from);
    piece_bbs[rook] &= ~(1ULL << rook_from);
    piece_bbs[king] |= (1ULL << king_to);
    piece_bbs[rook] |= (1ULL << rook_to);
}

void BoardState::castle_kingside()
{
    int rook_from = state.side_to_move == White ? h1 : h8;
    int rook_to = state.side_to_move == White ? f1 : f8;
    int king_from = state.side_to_move == White ? e1 : e8;
    int king_to = state.side_to_move == White ? g1 : g8;
    
    do_castle(rook_from, rook_to, king_from, king_to);
}

void BoardState::uncastle_kingside()
{
    int rook_from = state.side_to_move == White ? h1 : h8;
    int rook_to = state.side_to_move == White ? f1 : f8;
    int king_from = state.side_to_move == White ? e1 : e8;
    int king_to = state.side_to_move == White ? g1 : g8;
    
    do_castle(rook_to, rook_from, king_to, king_from);
}

void BoardState::castle_queenside()
{
    int rook_from = state.side_to_move == White ? a1 : a8;
    int rook_to = state.side_to_move == White ? c1 : c8;
    int king_from = state.side_to_move == White ? e1 : e8;
    int king_to = state.side_to_move == White ? d1 : d8;

    do_castle(rook_from, rook_to, king_from, king_to);
}

void BoardState::uncastle_queenside()
{
    int rook_from = state.side_to_move == White ? a1 : a8;
    int rook_to = state.side_to_move == White ? c1 : c8;
    int king_from = state.side_to_move == White ? e1 : e8;
    int king_to = state.side_to_move == White ? d1 : d8;

    do_castle(rook_to, rook_from, king_to, king_from);
}

// Remove from, add to
void BoardState::move_piece(int from, int to)
{
    Piece p = squares[from];
    /* if(p == -1) { */
    /*     std::cout << "p == -1 on move_piece: " */ 
    /*         << conversions::square_to_str(from) */
    /*         << conversions::square_to_str(to) */ 
    /*         << '\n'; */
    /*     assert(p != -1); */
    /* } */
    Colour pc = get_piece_colour(p);
    squares[from] = None;
    squares[to] = p;
    piece_bbs[p] &= ~(1ULL << from); 
    piece_bbs[p] |= (1ULL << to); 
    occ &= ~(1ULL << from); 
    occ |= (1ULL << to); 
    if(pc == White) {
        white_occ &= ~(1ULL << from); 
        white_occ |= (1ULL << to); 
    } else {
        black_occ &= ~(1ULL << from); 
        black_occ |= (1ULL << to); 
    }
}

bool BoardState::board_ok()
{
    // No extras in white_occ
    if((white_occ | occ) != occ) {
        std::cout << "\nwhite_occ"; 
        temp_print(white_occ);
        std::cout << "\nocc"; 
        temp_print(occ);
        std::cout << "\nextras"; 
        temp_print((white_occ & ~occ) );
        return false;
    }
    // No extras in black_occ
    if((black_occ | occ) != occ) {
        std::cout << "\nblack_occ"; 
        temp_print(black_occ);
        std::cout << "\nocc"; 
        temp_print(occ);
        std::cout << "\nextras"; 
        temp_print((black_occ & ~occ) );
        return false;
    }
    return true;
}

void BoardState::remove_piece(int sq)
{
    Piece p = squares[sq];
    /* if(p == -1) { */
    /*     std::cout << "p == -1 on remove_piece" */ 
    /*         << conversions::square_to_str(sq) */
    /*         << '\n'; */
    /*     assert(p != -1); */
    /* } */
    Colour pc = get_piece_colour(p);
    squares[sq] = None;
    piece_bbs[p] &= ~(1ULL << sq); 
    occ &= ~(1ULL << sq); 
    if(pc == White) {
        white_occ &= ~(1ULL << sq); 
    } else {
        black_occ &= ~(1ULL << sq); 
    }
}

void BoardState::put_piece(int sq, Piece p)
{
    /* assert(p != -1); */
    Colour pc = get_piece_colour(p);
    squares[sq] = p;
    piece_bbs[p] |= (1ULL << sq); 
    occ |= (1ULL << sq); 
    if(pc == White) {
        white_occ |= (1ULL << sq); 
    } else {
        black_occ |= (1ULL << sq); 
    }
}

// Assume legal
void BoardState::make_move(BMove m) 
{
    BMove from = (m >> 10) & 0x3f;
    BMove to = (m >> 4) & 0x3f;
    Move flag = (Move)(m & 0xf);
    Piece ep_pawn = None;
    int ep_square = -1;
    int ep_rank;

    Piece p = squares[from];
    int pt = conversions::piece_to_piecetype(p);
    Piece cp = squares[to];
    Colour us = state.side_to_move;

    // save state in prev_state 
    prev_state = state;

    // Reset ep_file
    state.ep_file = -1; 
        
    if(
        (flag & ~CHECK) != OOO // OOO (3) also intersects DOUBLE_PAWN_PUSH
    && (flag & DOUBLE_PAWN_PUSH)
    && ( (from >> 3 == 1 && us == White) || (from >> 3 == 7 && us == Black) )
    ) {
        state.ep_file = from % 8;
    } 
    
    if(flag & CAPTURE) { // Capture regular or capture En Passant
        int capsq = to;
        // En Passant
        if((flag & ~CHECK) == EN_PASSANT) {
            if(state.side_to_move == White) {
                ep_rank = 5;
                ep_pawn = BP;
                ep_square = ep_rank*8 + state.ep_file;
            } else {
                ep_rank = 4;
                ep_pawn = WP;
                ep_square = ep_rank*8 + state.ep_file;
            }
            capsq = ep_square;
            cp = ep_pawn;
        } 

        // Capture
        remove_piece(capsq);
        // Set captured piece
        state.last_captured = cp;
    } 

    if((flag & ~CHECK) == OO) {
        castle_kingside();
    } else if((flag & ~CHECK) != OOO) {
        castle_queenside();
    } else {
        move_piece(from, to); 
    }

    // Update board state
    if(p == BK) {
        state.b_castle_ks = false;
        state.b_castle_qs = false;
    }
    if(p == WK) {
        state.w_castle_ks = false;
        state.w_castle_qs = false;
    }
    if(p == BR) {
        if(from == h8)
            state.b_castle_ks = false;
        if(from == a8)
            state.b_castle_qs = false;
    } else if(p == WR) {
        if(from == h1)
            state.w_castle_ks = false;
        else if(from == a1)
            state.w_castle_qs = false;
    }
    if(state.side_to_move == Black) 
        state.ply_count++;
    if(pt != Pawn && flag != CAPTURE && flag != EN_PASSANT)
        state.halfmove_clock++;
    else 
        state.halfmove_clock = 0;

    state.side_to_move = static_cast<Colour>(!(bool)state.side_to_move); 
   
    // Add move to ongoing movelist
    movelist[movelist_idx] = m;
    movelist_idx++;

    // Make sure all is well afterwords 
    if(!board_ok()) {
        std::cout << "last move: "
            << conversions::square_to_str(from)
            << conversions::square_to_str(to)
            << ", " << conversions::flag_to_str(flag)
            << '\n';
        assert(board_ok());
    }
}

void BoardState::unmake_move(BMove m)
{
    int from = (m >> 10) & 0x3f;
    int to = (m >> 4) & 0x3f;
    int flag = m & 0xf;
    Piece p = squares[to]; 
    Piece cp = state.last_captured;

    state = prev_state;
    
    // Uncastle
    if((flag & ~CHECK) == OO) {
        uncastle_kingside(); 
    } else if((flag & ~CHECK) == OOO) {
        uncastle_queenside();
    } else {
        move_piece(to, from); // Castles doesn't have from and to squares right now
    }
    
    if(flag & CAPTURE) {
        // uncapture 
        put_piece(to, cp);
    } 

    if(!board_ok()) {
        std::cout << "last move: "
            << conversions::square_to_str(from)
            << conversions::square_to_str(to)
            << ", " << conversions::flag_to_str((flag & ~CHECK))
            << " checker: " << (bool)(flag & CHECK) 
            << " " << conversions::piece_to_str(p)
            << "x" << conversions::piece_to_str(cp) 
            << '\n';
        std::cout << "\nmoved piece: "
            << conversions::piece_to_str(p);
        temp_print(piece_bbs[p]);

        if(cp != None) {
            std::cout << "\ncaptured piece: " 
                << conversions::piece_to_str(cp);
            temp_print(piece_bbs[cp]);
        }
        assert(board_ok());
    }
}

Bitboard BoardState::get_friend_occ()
{
    return state.side_to_move == White ? white_occ : black_occ;
}

Bitboard BoardState::get_friend_occ(Colour us)
{
    return us == White ? white_occ : black_occ;
}

Bitboard BoardState::get_op_occ()
{
    return state.side_to_move == Black ? white_occ : black_occ;
}

Bitboard BoardState::get_op_occ(Colour us)
{
    return us == Black ? white_occ : black_occ;
}

Bitboard BoardState::get_side_piece_bb(int pt, Colour side)
{
    int p = side == White ? pt + 6 : pt;
    if(p >= 0 && p < 12)
        return piece_bbs[p];
    else {
        std::cerr << "----> get_friend_piece_bb(), Invalid piece input: " << p << '\n';
        return 0ULL;
    }
}

Bitboard BoardState::get_friend_piece_bb(int pt) 
{
    int p = state.side_to_move == White ? pt + 6 : pt;
    if(p >= 0 && p < 12)
        return piece_bbs[p];
    else {
        std::cerr << "----> get_friend_piece_bb(), Invalid piece input: " << p << '\n';
        return 0ULL;
    }
}

Bitboard BoardState::get_op_piece_bb(int pt) 
{
    int p = state.side_to_move == White ? pt : pt + 6 ;
    if(p >= 0 && p < 12)
        return piece_bbs[p];
    else {
        std::cerr << "----> get_op_piece_bb(), Invalid piece input: " << p << '\n';
        return 0ULL;
    }
}

Colour BoardState::get_piece_colour(Piece p)
{
    /* assert(p != -1); */
    if(p >= BQ && p <= BP)
        return Black;
    else if(p >= WQ && p <= WP)
        return White;
}

Bitboard BoardState::get_occ()
{
    return occ;
}

bool BoardState::in_check(Colour us)
{
    BMove prev_move;
    BMove prev_prev_move;
    Move pflag = (Move)(prev_move & 0xf);
    Move ppflag = (Move)(prev_prev_move & 0xf);
    if(us == state.side_to_move) {
        if(movelist_idx > 0)
            prev_move = movelist[movelist_idx - 1];
        else return false;
        if(pflag & CHECK)
            return true;
        return false;
    } 
    if(movelist_idx > 1)
        prev_prev_move = movelist[movelist_idx - 2];
    else return false;
    if(ppflag & CHECK)
        return true;
    return false;
}

bool is_piece_ch(char ch) 
{
    return (
            ch == 'p'
         || ch == 'r'
         || ch == 'q'
         || ch == 'k'
         || ch == 'n'
         || ch == 'b'
         || ch == 'P'
         || ch == 'R'
         || ch == 'Q'
         || ch == 'K'
         || ch == 'N'
         || ch == 'B'
    );
}


Piece fen_to_piece(char ch) 
{
    switch(ch) {
        case 'p': 
            return BP;
        case 'r':
            return BR;
        case 'q':
            return BQ;
        case 'k':
            return BK;
        case 'n':
            return BN;
        case 'b':
            return BB;
        case 'P':
            return WP;
        case 'R':    
            return WR;
        case 'Q':    
            return WQ;
        case 'K':    
            return WK;
        case 'N':    
            return WN;
        case 'B':    
            return WB;
        default: break;
    }
    return None;
}
