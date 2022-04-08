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


void print_squares(Piece squares[64])
{
    for(int y = 7; y >= 0; --y) {
        std::cout << "\n";
        for(int x = 0; x < 8; ++x) {
            int s = y*8 + x;
            if(squares[s] == -1) {
                std::cout << "|##";
            } else {
                std::cout << "|" << piece_to_str(squares[s]);
            }
            if(s == h8)
                std::cout << "|";
        }

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
                    state.ep_square = -1;
                } else if(info[i] >= 'a' && info[i] <= 'h') {
                    int ep_file = static_cast<int>(info[i] - 0x61);
                    int ep_rank = state.side_to_move == White ? 4 : 5;
                    state.ep_square = square(ep_rank, ep_file);
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
            int pt = piece_to_piecetype(p);
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
    init_attacks();
}

bool BoardState::can_castle(Colour us, Move type) 
{
    return true;
}

void BoardState::do_castle(int rook_from, int rook_to, int king_from, int king_to)
{
    Piece rook = state.side_to_move == White ? WR : BR;
    Piece king = state.side_to_move == White ? WK : BK;

    move_piece(rook_from, rook_to, rook);
    move_piece(king_from, king_to, king);
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
void BoardState::move_piece(int from, int to, Piece p)
{
    if(p == -1) {
        std::cout << "p == -1 on move_piece: " 
            << square_to_str(from)
            << square_to_str(to) 
            << '\n';
        assert(p != -1);
    }
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

void BoardState::move_piece(int from, int to) 
{
    move_piece(from, to, squares[from]);
}

bool BoardState::board_ok()
{
    // No extras in white_occ
    if((white_occ | occ) != occ) {
        return false;
    }
    // No extras in black_occ
    if((black_occ | occ) != occ) {
        return false;
    }
    return true;
}

void BoardState::remove_piece(int sq)
{
    Piece p = squares[sq];
    if(p == -1) {
        std::cout << "p == -1 on remove_piece" 
            << " " << square_to_str(sq)
            << '\n';
        print_previous_moves();
        print_squares(squares);
        print_occupied();
        assert(p != -1);
    }
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
    assert(p != -1);
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

void print_move(BMove m)
{
    BMove from = get_from(m);
    BMove to = get_to(m);
    std::cout 
            << square_to_str(from)
            << square_to_str(to);
}

void BoardState::print_previous_moves()
{
    std::cout << " Previous moves: ";
    std::for_each( 
            movelist.rbegin(),
            movelist.rend(), 
            [](const auto &m) {
                print_move(m);
                std::cout << " ";
    });
    std::cout << '\n';
}

void BoardState::print_occupied()
{
    std::cout << "occ";
    print(occ);
    std::cout << "white occ";
    print(white_occ);
    std::cout << "black occ";
    print(black_occ);
    std::cout << "black occ & ~occ";
    print(black_occ & ~occ);
    std::cout << "white occ & ~occ";
    print(white_occ & ~occ);
}

void BoardState::print_context(BMove m, bool capture, Move flag)
{
    BMove from = get_from(m);
    BMove to = get_to(m);

    if(flag == EN_PASSANT)
        std::cout << "ep square: " << square_to_str(get_ep_square());

    std::cout 
        << " On move: "
        << square_to_str(from)
        << square_to_str(to)
        << " type: " << flag_to_str(flag) 
        << " capture: " << (capture ? " yes " : " no ") << '\n'
        << " squares(to) " << piece_to_str(get_piece(to))
        << " squares(from) " << piece_to_str(get_piece(from)) << '\n';

    print_previous_moves();
    print_squares(squares);
    print_occupied();
}

// Assume legal
void BoardState::make_move(BMove m) 
{
    int from = get_from(m);
    int to = get_to(m);
    Move flag = get_flag(m);
    Piece p = get_piece(from);
    int pt = piece_to_piecetype(p);
    Piece cp = get_piece(to);
    bool capture = ((cp != None) || (flag == EN_PASSANT));
    /* bool capture = cp != None; */
    Colour us = state.side_to_move;

    assert(p != -1);

    // save state in prev_state 
    prev_state = state;

    state.last_captured = None; 

    bool ep_possible = (
        flag == DOUBLE_PAWN_PUSH && 
        (
            (get_op_piece_bb(Pawn) & bit(to + W) & ~FileHBB) ||
            (get_op_piece_bb(Pawn) & bit(to + E) & ~FileABB)   
        )
    );

    // Set En Passant square
    if(ep_possible) 
        state.ep_square = to;
    else state.ep_square = -1;  

    // Capture regular or En Passant
#if 1 
    if(capture) { 
        int capsq = to;
        // En Passant
        if(flag == EN_PASSANT) {
            capsq = to + (us == White ? S : N);
            cp = us == White ? BP : WP;
        } 

        // Capture
        remove_piece(capsq);
        state.last_captured = cp; 
    } 
#endif

    if(flag == OO) {
        castle_kingside();
    } else if(flag == OOO) {
        castle_queenside();
    } else {
        move_piece(from, to); 
    }

    // Update board state
    // Reset captured piece and ep square
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
    if(pt != Pawn && !capture && flag != EN_PASSANT)
        state.halfmove_clock++;
    else 
        state.halfmove_clock = 0;

    state.side_to_move = static_cast<Colour>(!(bool)state.side_to_move); 
   
    // Add move to ongoing movelist
    movelist.push_back(m);

    // TEMP
    /* print_squares(squares); */

    // Make sure all is well afterwords 
    if(!board_ok()) {
        std::cout << "on make move" << '\n';
        print_context(m, capture, flag);
        assert(board_ok());
    }
}

void BoardState::unmake_move(BMove m)
{
    int from = get_from(m);
    int to = get_to(m);
    Move flag = get_flag(m);
    Piece p = get_piece(to); 

    Piece cp; 
    bool capture;
    Colour us;

    if(p == -1 && flag != OO && flag != OOO) {
        std::cout 
            << square_to_str(from)
            << square_to_str(to)
            << '\n';
        assert(p != -1);
    }

    state = prev_state;

    cp = state.last_captured;
    capture = (cp != None);
    us = state.side_to_move;
    
    // Uncastle
    if(flag == OO) {
        uncastle_kingside(); 
    } else if(flag == OOO) {
        uncastle_queenside();
    } else {
        move_piece(to, from); 
    }
    
    if(capture) {
        int capsq = to;
        if(flag == EN_PASSANT) {
            capsq = get_ep_square();
        }
        // uncapture 
        put_piece(capsq, cp);
    } 

    if(!board_ok()) {
        std::cout << "on unmake move" << '\n';
        print_context(m, capture, flag);
        assert(board_ok());
    }
    movelist.pop_back();
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
    assert(p != None);
    if(p >= BQ && p <= BP)
        return Black;
    return White;
}

Bitboard BoardState::get_occ()
{
    return occ;
}

void BoardState::init_behind()
{
    for(int f=a1; f<=h8; ++f) {
        for(int di=0; di<8; ++di) { 
            Direction d = directions[di];
            for(int t=f+d; in_bounds(t, d); t+=d) {
                behind[f][t] = trace_ray(t,d);
            }
        }
    }
}

void BoardState::init_piece_attacks()
{
    for(int ss=a1; ss<=h8; ++ss) {
        Square sq = (Square)ss;
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

void BoardState::init_attacks() 
{
    init_behind();
    init_piece_attacks();
}


// Returns attacks squares of a (not pawn) piece on a square 
Bitboard BoardState::blockers_and_beyond(int p, int from)
{
    Bitboard ts = piece_attacks[p][from];
    Bitboard bb = occ;
    while(bb) {
        int to = pop_bit(bb);
        ts &= ~behind[from][to];
    }
    
    return ts;
}

// Returns pawns attacks squares
// Double pushes and En Passant are handled directly in generator function in order to also attach relevant flag
Bitboard BoardState::pawn_squares(int origin, Colour us) 
{
    Direction push_dir = us == White ? N : S;  

    Bitboard ratt = bit(origin + push_dir + E) & ~FileABB;
    Bitboard latt = bit(origin + push_dir + W) & ~FileHBB;
    Bitboard single_push = bit(origin + push_dir);

    Bitboard mask = ratt | latt | single_push;
    Bitboard squares = 0; 

    Bitboard op_occ = get_op_occ();
    Bitboard friend_occ = get_friend_occ();

    // Captures 
    if(op_occ & ratt) {
        squares |= ratt;
    }
    if(op_occ & latt) {
        squares |= latt;
    }

    // Single push
    if(!(single_push & op_occ)) {
        squares |= single_push;
    }

    return squares & mask & ~friend_occ;
}

int BoardState::get_ep_square()
{
    return state.ep_square; 
}

Bitboard BoardState::attacks_to(int sq, Colour attacker)
{
    return 0ULL;
}

// Returns attacks of a given piece on a given square 
Bitboard BoardState::get_to_squares(int p, int from, Colour us)
{
    Bitboard ts = 0ULL; 
    Bitboard friend_occ = get_friend_occ(us);
    if(p == Pawn) {
        ts = pawn_squares(from, us);
    } else {
        ts = blockers_and_beyond(p, from);
    }
    ts &= ~friend_occ;
    return ts;
}

bool BoardState::in_check(Colour us)
{
    return false; 
}

Piece BoardState::get_piece(int s)
{
    return squares[s];
}

bool BoardState::operator==(BoardState b)
{
    for(int p=BQ; p <= WP; ++p) {
        if(this->piece_bbs[p] != b.piece_bbs[p])
            return false;
    }
    return this->white_occ == b.white_occ
        && this->black_occ == b.black_occ  
        && this->occ == b.occ
        && this->state == b.state;
}

bool State::operator==(State &b)
{
  return this->side_to_move   == b.side_to_move
      && this->w_castle_ks    == b.w_castle_ks
      && this->w_castle_qs    == b.w_castle_qs
      && this->b_castle_ks    == b.b_castle_ks 
      && this->b_castle_qs    == b.b_castle_qs
      && this->ep_square        == b.ep_square
      && this->halfmove_clock == b.halfmove_clock
      && this->ply_count      == b.ply_count
      && this->last_captured  == b.last_captured;
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
