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

void BoardState::print_squares() const
{
    for(int y = 7; y >= 0; --y) {
        std::cout << "\n";
        for(int x = 0; x < 8; ++x) {
            Square s = (Square)(y*8 + x);
            if(squares[s] == None) {
                if(!((y + x) % 2))
                    std::cout << "|##";
                else 
                    std::cout << "|  ";
            } else {
                std::cout << "|" << piece_to_str(squares[s]);
            }
            if(file(s) == 7)
                std::cout << "|";
        }

    }
    std::cout << "\n";
}

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
    for(size_t i=0; i<info.length(); ++i) {
        if(info[i] == ' ') {
            section++;
            /* continue; */
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
                    state.ep_square = NullSquare;
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
    // FIXME: int -> Piece ?
    for(int p=BQ; p<=WP; ++p) {
        piece_bbs[p] = 0ULL;
    }

    for(Square s = a1; s <= h8; s = s + E) {
        Piece p = squares[s];
        if(p != None) { 
            set_bit(piece_bbs[p], s);
            set_bit(occ, s);
            if(get_piece_colour(p) == White) {
                set_bit(white_occ, s);
            } else {
                set_bit(black_occ, s);
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

bool BoardState::can_castle(Colour us, Move type) const
{
    assert(type == OO || type == OOO);

    bool has_right = ([&]() -> bool {
        if(type == OO) {
            if(us == White) return state.w_castle_ks;
            return state.b_castle_ks;
        } else {
            if(us == White) return state.w_castle_qs;
            return state.b_castle_qs;
        }
    })();

    if(!has_right) 
        return false;

    if(in_check(us)) 
        return false;

    // If there is at least one attacker attacking a square between rook and king
    Bitboard in_between = ([&]() -> Bitboard {
        if(type == OO) {
            if(us == White) return bit(f1) | bit(g1);
            return bit(f8) | bit(g8);
        } else {
            if(us == White) return bit(d1) | bit(c1);
            return bit(d8) | bit(c8);
        }
    })();

    while(in_between) {
        Square between_square = pop_bit(in_between);
        if(attacked(between_square, !us)) 
            return false;
    }

    // If there are any pieces in between the rook and king
    Square rooksq = ([&]() -> Square { 
        if(type == OO) {
            if(us == White) return h1;
            return h8;
        } else {
            if(us == White) return a1;
            return a8;
        } 
    })();

    Square kingsq = get_king_square(us);
    Bitboard kingbb = get_side_piece_bb(King, us);
    Bitboard file_mask = type == OO ? FileHBB : FileABB;
    Bitboard mask = file_mask | behind[rooksq][kingsq] | kingbb;

    Bitboard has_blockers = piece_attacks[Rook][rooksq] & occ & ~mask;
    if(has_blockers) 
        return false;

    return true;
}

void BoardState::do_castle(Square rook_from, Square rook_to, Square king_from, Square king_to)
{
    if(squares[rook_from] == None || squares[king_from] == None) {
        std::cout << "failed to castle \n";
        print_squares();
        print_moves();
        assert(squares[rook_from] != None && squares[king_from] != None);
    }
    Piece rook = state.side_to_move == White ? WR : BR;
    Piece king = state.side_to_move == White ? WK : BK;

    move_piece(rook_from, rook_to, rook);
    move_piece(king_from, king_to, king);
}

void BoardState::castle_kingside()
{
    Square rook_from = state.side_to_move == White ? h1 : h8;
    Square rook_to = state.side_to_move == White ? f1 : f8;
    Square king_from = state.side_to_move == White ? e1 : e8;
    Square king_to = state.side_to_move == White ? g1 : g8;
    
    do_castle(rook_from, rook_to, king_from, king_to);
}

void BoardState::uncastle_kingside()
{
    Square rook_from = state.side_to_move == White ? h1 : h8;
    Square rook_to = state.side_to_move == White ? f1 : f8;
    Square king_from = state.side_to_move == White ? e1 : e8;
    Square king_to = state.side_to_move == White ? g1 : g8;
    
    do_castle(rook_to, rook_from, king_to, king_from);
}

void BoardState::castle_queenside()
{
    Square rook_from = state.side_to_move == White ? a1 : a8;
    Square rook_to = state.side_to_move == White ? d1 : d8;
    Square king_from = state.side_to_move == White ? e1 : e8;
    Square king_to = state.side_to_move == White ? c1 : c8;

    do_castle(rook_from, rook_to, king_from, king_to);
}

void BoardState::uncastle_queenside()
{
    Square rook_from = state.side_to_move == White ? a1 : a8;
    Square rook_to = state.side_to_move == White ? d1 : d8;
    Square king_from = state.side_to_move == White ? e1 : e8;
    Square king_to = state.side_to_move == White ? c1 : c8;

    do_castle(rook_to, rook_from, king_to, king_from);
}

// Remove from, add to
void BoardState::move_piece(Square from, Square to, Piece p)
{
    assert(p != None);
    put_piece(to, p);
    remove_piece(from);
}

void BoardState::move_piece(Square from, Square to) 
{
    Piece p = squares[from];
    assert(p != None);
    move_piece(from, to, p);
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
    // TODO: Check there are not more than 12 pieces on the board 

    return true;
}

void BoardState::remove_piece(Square sq)
{
    Piece p = squares[sq];

    if(p == None) {
        std::cout << "failed to remove piece on " << square_to_str(sq) << '\n';
        print_squares();
        print_moves();

        assert(p != None);
    }

    Colour pc = get_piece_colour(p);
    squares[sq] = None;
    clear_bit(piece_bbs[p], sq); 
    clear_bit(occ, sq); 
    if(pc == White) {
        clear_bit(white_occ, sq); 
    } else {
        clear_bit(black_occ, sq); 
    }
}

void BoardState::put_piece(Square sq, Piece p)
{
    assert(p != None && sq != NullSquare);
    Colour pc = get_piece_colour(p);
    squares[sq] = p;
    set_bit(piece_bbs[p], sq); 
    set_bit(occ, sq); 
    if(pc == White) {
        set_bit(white_occ, sq); 
    } else {
        set_bit(black_occ, sq); 
    }
}

// NOTE: Do promotion only after pawn has moved
void BoardState::promote(PieceType pt, Square sq, Colour us) 
{
    assert(pt != King);
    assert(pt != Pawn);
    assert(rank(sq) == get_opposite_end(us));

    Piece p = piecetype_to_piece(pt, us);
    remove_piece(sq);
    put_piece(sq, p); // Put the promoted piece on the square
    /* assert(0); */
}

void BoardState::print_move(BMove m) const
{
    BMove from = get_from(m);
    BMove to = get_to(m);
    
    std::cout 
            << square_to_str(from)
            << square_to_str(to);
}

void BoardState::print_moves() const
{
    for(size_t i = 0; i < movelist.size(); ++i) {
        std::cout << get_algebraic(movelist[i]) << " ";  
    }
    std::cout << '\n';
}

void BoardState::print_occupied() const
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

void BoardState::print_context(BMove m, bool capture, Move flag) const
{
    Square from = get_from(m);
    Square to = get_to(m);

    if(flag == EN_PASSANT)
        std::cout << "ep square: " << square_to_str(get_ep_square());

    std::cout 
        << "\n----------------------------------\n"
        << "Printing context... \n" 
        << "from: " << square_to_str(from) 
        << " to: " << square_to_str(to) 
        << " type: " << flag_to_str(flag) 
        << " capture: " << (capture ? " yes " : " no ") << '\n'
        << " squares(to) " << piece_to_str(get_piece(to))
        << " squares(from) " << piece_to_str(get_piece(from)) << '\n';

    print_squares();
    /* print_occupied(); */
    std::cout << "\n----------------------------------\n"; 
}


// Assume legal
void BoardState::make_move(BMove m) 
{
    Square from = get_from(m);
    Square to = get_to(m);
    Move flag = get_flag(m);
    Piece p = get_piece(from);
    PieceType pt = piece_to_piecetype(p);
    Piece cp = get_piece(to);
    bool capture = ((cp != None) || (flag == EN_PASSANT));
    Colour us = state.side_to_move;

    if(p == None) 
        print_context(m, capture, flag); 
    assert(p != None);

    // save state in prev_state 
    prev_state = state;

    state.last_captured = cp;  // ??

    // En Passant Possibility
    bool pawn_adj = 
        (get_op_piece_bb(Pawn) & bit(to + W) & ~FileHBB) ||
        (get_op_piece_bb(Pawn) & bit(to + E) & ~FileABB);

    bool ep_possible = flag == DOUBLE_PAWN_PUSH && pawn_adj;

    // Set En Passant square
    if(ep_possible) 
        state.ep_square = to;
    else state.ep_square = NullSquare;  

    // Capture regular or En Passant
    Square capsq = NullSquare;
    if(capture) { 
         capsq = to;
        // En Passant
        if(flag == EN_PASSANT) {
            capsq = to + (us == White ? S : N);
            cp = us == White ? BP : WP;
        } 

        // Capture
        remove_piece(capsq);
        state.last_captured = cp; 
    } 

    // Move the piece
    if(flag == OO) {
        castle_kingside();
    } else if(flag == OOO) {
        castle_queenside();
    } else {
        move_piece(from, to, p); 
    }

    // Promotions
    if(pt == Pawn && flag >= PROMOTE_QUEEN && flag <= PROMOTE_BISHOP) {
        switch(flag) {
            case PROMOTE_QUEEN:
                promote(Queen, to, us);
                break;
            case PROMOTE_BISHOP:
                promote(Bishop, to, us);
                break;
            case PROMOTE_ROOK:
                promote(Rook, to, us);
                break;
            case PROMOTE_KNIGHT:
                promote(Knight, to, us);
                break;
            default: break;
        }
    }

    // Add move to ongoing movelist
    movelist.push_back(m);

    // Update board state
    // Is rook or king move 
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
    if(us == White && cp == BR && to == h8)
        state.b_castle_ks = false;
    else if(us == White && cp == BR && to == a8)
        state.b_castle_qs = false;
    else if(us == Black && cp == WR && to == h1) 
        state.w_castle_ks = false;
    else if(us == Black && cp == WR && to == a1) 
        state.w_castle_qs = false;
    if(state.side_to_move == Black) 
        state.ply_count++;
    if(pt != Pawn && !capture && flag != EN_PASSANT)
        state.halfmove_clock++;
    else 
        state.halfmove_clock = 0;

    state.side_to_move = static_cast<Colour>(!(bool)state.side_to_move); 

    // Make sure all is well afterwords 
    if(!board_ok()) {
        std::cout << "on make move" << '\n';
        print_context(m, capture, flag);
        assert(board_ok());
    }
}

void BoardState::unmake_move(BMove m)
{
    Square from = get_from(m);
    Square to = get_to(m);
    Move flag = get_flag(m);
    Piece p = get_piece(to); 

    Piece cp = state.last_captured; 
    state = prev_state;
    bool capture = (cp != None);
    Square capsq = to;
    Colour us = get_side_to_move();

    if(p == None) {
        print_context(m, capture, flag); 
        assert(p != None);
    }


    // Unpromote
    if(flag >= PROMOTE_QUEEN && flag <= PROMOTE_BISHOP) {
        remove_piece(to);
        put_piece(to, piecetype_to_piece(Pawn, us));
    }

    // Uncastle
    if(flag == OO) {
        uncastle_kingside(); 
    } else if(flag == OOO) {
        uncastle_queenside();
    } else {
        // Unmove piece if its not a castle
        move_piece(to, from, p); 
    }
    
    if(capture) {
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

// FIXME: Return get_friend_occ(state.side_to_move)
Bitboard BoardState::get_friend_occ() const
{
    return state.side_to_move == White ? white_occ : black_occ;
}

Bitboard BoardState::get_friend_occ(Colour us) const
{
    return us == White ? white_occ : black_occ;
}

// FIXME: Return get_occ_occ(state.side_to_move)
Bitboard BoardState::get_op_occ() const 
{
    return state.side_to_move == Black ? white_occ : black_occ;
}

Bitboard BoardState::get_op_occ(Colour us) const 
{
    return us == Black ? white_occ : black_occ;
}

Bitboard BoardState::get_side_piece_bb(int pt, Colour side) const 
{
    int p = side == White ? pt + 6 : pt;
    if(p >= 0 && p < 12)
        return piece_bbs[p];
    else {
        std::cerr << "----> get_friend_piece_bb(), Invalid piece input: " << p << '\n';
        return 0ULL;
    }
}

Bitboard BoardState::get_friend_piece_bb(int pt) const 
{
    int p = state.side_to_move == White ? pt + 6 : pt;
    if(p >= 0 && p < 12)
        return piece_bbs[p];
    else {
        std::cerr << "----> get_friend_piece_bb(), Invalid piece input: " << p << '\n';
        return 0ULL;
    }
}

Bitboard BoardState::get_op_piece_bb(int pt) const
{
    int p = state.side_to_move == White ? pt : pt + 6 ;
    if(p >= 0 && p < 12)
        return piece_bbs[p];
    else {
        std::cerr << "----> get_op_piece_bb(), Invalid piece input: " << p << '\n';
        return 0ULL;
    }
}

Colour BoardState::get_piece_colour(Piece p) const
{
    assert(p != None);
    if(p >= BQ && p <= BP)
        return Black;
    return White;
}

void BoardState::init_behind()
{
    for(Square f=a1; f<=h8; f = f + E) {
        for(int di=0; di<8; ++di) { 
            Direction d = directions[di];
            for(Square t=f+d; in_bounds(t, d); t = t + d) {
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
inline Bitboard BoardState::blockers_and_beyond(PieceType pt, Square from) const
{
    Bitboard ts = piece_attacks[pt][from];
    Bitboard bb = occ;
    while(bb) {
        Square to = pop_bit(bb);
        ts &= ~behind[from][to];
    }
    return ts;
}

// Returns pawns attacks squares
// Double pushes and En Passant are handled directly in generator function in order to also attach relevant flag
inline Bitboard BoardState::pawn_squares(Square origin, Colour us) const
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

Square BoardState::get_ep_square() const 
{
    return state.ep_square; 
}

Square BoardState::get_king_square(Colour us) const 
{
    const Bitboard king_bb = get_side_piece_bb(King, us);
    // FIXME 
    if(popcount(king_bb) != 1) {
        print(king_bb);
        print_squares();
        print_moves();
        assert(popcount(king_bb) == 1);
    }
    return lsb(king_bb);
}

Bitboard BoardState::attacks_to(Square sq) const
{
    //  - Maybe we can cache to squares on each move made 
    // Naive implementation
    // Build a hypothetical piece which has all piece attacks
    // place hypothetical piece on square, get its attacks
    // Intersection of attacking pieces and super_piece_attacks
    
    const Bitboard queen_attacks = blockers_and_beyond(Queen, sq);
    const Bitboard bishop_attacks = blockers_and_beyond(Bishop, sq);
    const Bitboard rook_attacks = blockers_and_beyond(Rook, sq);
    
    Bitboard super_piece = 
          ( piece_attacks[King][sq] & get_piecetype_bb(King) )
        | ( piece_attacks[Knight][sq] & get_piecetype_bb(Knight) )
        | ( queen_attacks & get_piecetype_bb(Queen) )
        | ( bishop_attacks & get_piecetype_bb(Bishop) )
        | ( rook_attacks & get_piecetype_bb(Rook) )
        | ( pawn_captures_mask(sq, White) & get_piece_bb(BP) )
        | ( pawn_captures_mask(sq, Black) & get_piece_bb(WP) );

    return super_piece;
}

// Is a square attacked
// NOTE: Asking if a friendly occupied square is attacked (and it is) will return true
//       when using this function make sure that you are not asking if a friendly square is attacked by friends
// FIXME: Incrementally updated attack tables 
//        re calculating attacks is noticeably slow
bool BoardState::attacked(Square sq, Colour by) const
{
    Bitboard kings = get_side_piece_bb(King, by);
    Bitboard king_attacks = piece_attacks[King][sq];
    if(king_attacks & kings) 
        return true;

    Bitboard knights = get_side_piece_bb(Knight, by);
    Bitboard knight_attacks = piece_attacks[Knight][sq];
    if(knight_attacks & knights) 
        return true;

    Bitboard pawns = get_side_piece_bb(Pawn, by);
    Bitboard pawn_attacks = pawn_captures_mask(sq, !by);
    if(pawn_attacks & pawns) 
        return true;

    Bitboard bishops = get_side_piece_bb(Bishop, by);
    Bitboard bishop_attacks = blockers_and_beyond(Bishop, sq);
    if(bishop_attacks & bishops) 
        return true;

    Bitboard queens = get_side_piece_bb(Queen, by);
    Bitboard queen_attacks = blockers_and_beyond(Queen, sq);
    if(queen_attacks & queens) 
        return true;

    Bitboard rooks = get_side_piece_bb(Rook, by);
    Bitboard rook_attacks = blockers_and_beyond(Rook, sq);
    if(rook_attacks & rooks) 
        return true;

    return false;
}

// Returns attacks of a given piece on a given square 
Bitboard BoardState::get_to_squares(PieceType pt, Square from, Colour us) const 
{
    Bitboard friend_occ = get_friend_occ(us);
    if(pt == Pawn) {
        return pawn_squares(from, us);
    } 
    return blockers_and_beyond(pt, from) & ~friend_occ;
}

Bitboard BoardState::checkers(Colour us) const 
{
    return 0ULL;
}

bool BoardState::in_check(Colour us) const
{
    const Square kingsq = get_king_square(us);
    const bool king_attacked = attacked(kingsq, !us); 
    return king_attacked; 
}

// FIXME
bool BoardState::in_checkmate(Colour us) const 
{
    return in_check(us);
}

Piece BoardState::get_piece(Square s) const 
{
    if(s > h8 || s < a1) {
        printf("get_piece(%d)\n", s);
        return None;
    }

    return squares[s];
}

std::string BoardState::get_algebraic(BMove m) const 
{
    if(m == 0) return "";

    std::string algebraic = "";
    Square from = get_from(m);
    Square to = get_to(m);
    Move flag = get_flag(m);
    PieceType moved = piece_to_piecetype(get_piece(from));
    PieceType captured = piece_to_piecetype(get_piece(to));
    Colour us = get_side_to_move();
    bool check = in_check(!us);

    std::string tosq = square_to_str(to);
    if(flag == OO || flag == OOO) {
        algebraic = flag_to_str(flag);
    } else if(moved == Pawn) {
        if(captured == Null) 
            algebraic = tosq;
        else algebraic = "Px" + tosq;
        if(flag >= PROMOTE_QUEEN && flag <= PROMOTE_BISHOP)
            algebraic += promote_flag_to_str(flag);
    } else {
        algebraic = piecetype_to_algstr(moved) + (captured != Null ? "x" : "") + tosq;
    }

    if(check) 
        algebraic += "+";

    return algebraic;
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
