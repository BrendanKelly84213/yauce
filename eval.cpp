#include "eval.h"

const unsigned int P_weight = 100;
const unsigned int N_weight = 320;
const unsigned int B_weight = 330;
const unsigned int R_weight = 500;
const unsigned int Q_weight = 900;
const unsigned int K_weight = 20000;

const int w_pawn_table[] = {
0,  0,  0,  0,  0,  0,  0,  0,
50, 50, 50, 50, 50, 50, 50, 50,
10, 10, 20, 30, 30, 20, 10, 10,
 5,  5, 10, 25, 25, 10,  5,  5,
 0,  0,  0, 20, 20,  0,  0,  0,
 5, -5,-10,  0,  0,-10, -5,  5,
 5, 10, 10,-20,-20, 10, 10,  5,
 0,  0,  0,  0,  0,  0,  0,  0
};

const int w_knight_table[] = {
-50,-40,-30,-30,-30,-30,-40,-50,
-40,-20,  0,  0,  0,  0,-20,-40,
-30,  0, 10, 15, 15, 10,  0,-30,
-30,  5, 15, 20, 20, 15,  5,-30,
-30,  0, 15, 20, 20, 15,  0,-30,
-30,  5, 10, 15, 15, 10,  5,-30,
-40,-20,  0,  5,  5,  0,-20,-40,
-50,-40,-30,-30,-30,-30,-40,-50,
};

const int w_bishop_table[] = {
-20,-10,-10,-10,-10,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5, 10, 10,  5,  0,-10,
-10,  5,  5, 10, 10,  5,  5,-10,
-10,  0, 10, 10, 10, 10,  0,-10,
-10, 10, 10, 10, 10, 10, 10,-10,
-10,  5,  0,  0,  0,  0,  5,-10,
-20,-10,-10,-10,-10,-10,-10,-20,
};


const int w_rook_table[] = {
    0,  0,  0,  0,  0,  0,  0,  0,
  5, 10, 10, 10, 10, 10, 10,  5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
  0,  0,  0,  5,  5,  0,  0,  0
};

const int w_queen_table[] = {
-20,-10,-10, -5, -5,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5,  5,  5,  5,  0,-10,
 -5,  0,  5,  5,  5,  5,  0, -5,
  0,  0,  5,  5,  5,  5,  0, -5,
-10,  5,  5,  5,  5,  5,  0,-10,
-10,  0,  5,  0,  0,  0,  0,-10,
-20,-10,-10, -5, -5,-10,-10,-20
};

const int w_king_mg_table[] = {
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-20,-30,-30,-40,-40,-30,-30,-20,
-10,-20,-20,-20,-20,-20,-20,-10,
 20, 20,  0,  0,  0,  0, 20, 20,
 20, 30, 10,  0,  0, 10, 30, 20
};

const int w_king_eg_table[] = {
-50,-40,-30,-20,-20,-30,-40,-50,
-30,-20,-10,  0,  0,-10,-20,-30,
-30,-10, 20, 30, 30, 20,-10,-30,
-30,-10, 30, 40, 40, 30,-10,-30,
-30,-10, 30, 40, 40, 30,-10,-30,
-30,-10, 20, 30, 30, 20,-10,-30,
-30,-30,  0,  0,  0,  0,-30,-30,
-50,-30,-30,-30,-30,-30,-30,-50
};

int b_pawn_table[64];
int b_knight_table[64]; 
int b_bishop_table[64];
int b_rook_table[64];
int b_queen_table[64];
int b_king_mg_table[64];
int b_king_eg_table[64];

const int * white_mg_tables[] = {
    w_queen_table,
    w_king_mg_table,
    w_rook_table,
    w_knight_table,
    w_bishop_table,
    w_pawn_table
};

const int * white_eg_tables[] = {
    w_queen_table,
    w_king_eg_table,
    w_rook_table,
    w_knight_table,
    w_bishop_table,
    w_pawn_table
};

int black_mg_tables[6][64];

int black_eg_tables[6][64];

int piece_weights[] = {
    Q_weight,
    K_weight,
    R_weight,
    N_weight,
    B_weight,
    P_weight
};

int piece_weight(PieceType pt)
{
    if(pt == Null)
        return 0;
    return piece_weights[pt];
}

void init_black_tables()
{
    for(Square s = a1; s <= h8; s = s + E) {

        for(PieceType pt = Queen; pt <= Pawn; ++pt) {
            black_mg_tables[pt][s] = white_mg_tables[pt][(s ^ 56)];
            black_eg_tables[pt][s] = white_eg_tables[pt][(s ^ 56)];
        }
    }
}

void print_table(int table[])
{
    for(Square s = a1; s <= h8; s = s + E) {
        if(!(s % 8))
            std::cout << '\n';
        std::cout << table[s] << " "; 
    }
}
void print_black_tables()
{
    std::cout << "black pawn" << '\n';
    print_table(b_pawn_table );
    std::cout << "\nblack knight" << '\n';
    print_table(b_knight_table );
    std::cout << "\nblack bishop" << '\n';
    print_table(b_bishop_table );
    std::cout << "\nblack rook" << '\n';
    print_table(b_rook_table );
    std::cout << "\nblack queen" << '\n';
    print_table(b_queen_table );
} 

enum PiecePhase {
    NullPhase = -1, // Standin for King
    PawnPhase,
    BishopPhase = 1,
    KnightPhase = 1,
    RookPhase = 2,
    QueenPhase = 4
};

constexpr int TotalPhase = PawnPhase * 16 + KnightPhase * 4 + BishopPhase * 4 + RookPhase * 4 + QueenPhase * 2; 

    // Queen, King, Rook, Knight, Bishop, Pawn, Null=-1
const PiecePhase phase_table[] = {
    QueenPhase,
    NullPhase,
    RookPhase,
    KnightPhase,
    BishopPhase,
    PawnPhase 
};

int phase_weight(const BoardState &board)
{
    int phase = TotalPhase;
    for(PieceType pt = Queen; pt <= Pawn; ++pt) {
        if(pt != King) {
            size_t num_pieces = board.get_num_piecetype(pt);
            phase -= num_pieces * phase_table[pt];       
        }
    }     
    phase = (phase * 256 + (TotalPhase / 2)) / TotalPhase;
    return phase;
}

int endgame_weight(const BoardState &board)
{
    size_t num_pieces = board.get_total_piece_count();
    return 24 - num_pieces;
}

int relative_piece_count(const BoardState &board, Colour us)
{
    int piece_count = 0;
    for(Piece p = (us == White ? WQ : BQ); p <= (us == White ? WP : BP); ++p) {
        if(p != WK && p != BK) {
            size_t num_pieces = board.get_num_piece(p);
            piece_count += num_pieces * phase_table[piece_to_piecetype(p)];       
        }
    }     
    return piece_count;
}

int king_distance(const BoardState &board)
{
    Square wking_square = board.get_king_square(White);
    Square bking_square = board.get_king_square(Black);
    return distance(wking_square, bking_square); 
}

int king_distance_from_center(const BoardState &board, Colour us)
{
    Square kingsq = board.get_king_square(us);
    Square center_square;
    int king_rank = rank(kingsq);
    int king_file = file(kingsq);
    // Refactor
    if(king_rank >= 4 && king_file >= 4) {
        // King is in top right quadrant
        center_square = e5;
    } else if(king_rank >= 4 && king_file < 4) {
        // King is in top left quadrant
        center_square = d5;
    } else if(king_rank < 4 && king_file >=4) {
        // King is in bottom right quadrant
        center_square = e4;
    } else {
        center_square = d4;
    }

    int king_dist_from_center = distance(kingsq, center_square); 
    
    return king_dist_from_center;
}

int op_king_distance_from_center(const BoardState &board)
{
    Colour us = board.get_side_to_move();
    Square opkingsq = board.get_king_square(!us);
    Square center_square;
    int king_rank = rank(opkingsq);
    int king_file = file(opkingsq);
    // Refactor
    if(king_rank >= 4 && king_file >= 4) {
        // King is in top right quadrant
        center_square = e5;
    } else if(king_rank >= 4 && king_file < 4) {
        // King is in top left quadrant
        center_square = d5;
    } else if(king_rank < 4 && king_file >=4) {
        // King is in bottom right quadrant
        center_square = e4;
    } else {
        center_square = d4;
    }

    int opking_dist_from_center = distance(opkingsq, center_square); 
    
    return opking_dist_from_center;
}

size_t connected_pawns(Bitboard pawns, Square origin, Direction d)
{
    Square to = origin + d;
    size_t num_connected_pawns = 0;
    while(bit(to) & pawns) {
       num_connected_pawns++;  
       to = to + d; 
    }
    return num_connected_pawns;
}

int pawn_chain(const BoardState &board, Colour us) 
{
    Bitboard our_pawns = board.get_piece_bb(Pawn, us);
    int bonus = 0; 
    while(our_pawns) {
        Square s = pop_bit(our_pawns);
        bonus += connected_pawns(our_pawns, s, NE);
        bonus += connected_pawns(our_pawns, s, NW);
        bonus += connected_pawns(our_pawns, s, SE);
        bonus += connected_pawns(our_pawns, s, SW);
    }

    return bonus;
}

int passed_pawn(const BoardState &board, Colour us)
{
    int bonus = 0;
    Bitboard our_pawns = board.get_piece_bb(Pawn, us);
    Bitboard their_pawns = board.get_piece_bb(Pawn, us);
    while(our_pawns) {
       Square s = pop_bit(our_pawns);     
       Bitboard left_file = filebb(s + W);
       Bitboard right_file = filebb(s + E);
       Bitboard pawns_file = filebb(s);
       Bitboard defending_pawns = (pawns_file | left_file | right_file) & their_pawns;
       bonus += !(bool)defending_pawns; 
    }
    
    return bonus;
}

int mg_piece_weight(const BoardState &board, Piece p)
{
    int weight = 0;
    Bitboard piecebb = board.get_piece_bb(p);
    PieceType pt = piece_to_piecetype(p);
    Colour piece_colour = piece_to_colour(p);

    while(piecebb) {
        Square s = pop_bit(piecebb);

        if(piece_colour == White) {
            weight += piece_weights[pt] + white_mg_tables[pt][s]; 
        } else  {
            weight += piece_weights[pt] + black_mg_tables[pt][s]; 
        }
    }
    return weight;
}

int eg_piece_weight(const BoardState &board, Piece p)
{
    int weight = 0;
    Bitboard piecebb = board.get_piece_bb(p);
    PieceType pt = piece_to_piecetype(p);
    Colour piece_colour = piece_to_colour(p);

    while(piecebb) {
        Square s = pop_bit(piecebb);

        if(piece_colour == White) {
            weight += piece_weights[pt] + white_eg_tables[pt][s]; 
        } else  {
            weight += piece_weights[pt] + black_eg_tables[pt][s]; 
        }
    }

    return weight;
}

int piece_weight(const BoardState &board, Piece p)
{
    int weight = 0;
    bool endgame = endgame_weight(board) >= 16;
    Bitboard piecebb = board.get_piece_bb(p);
    PieceType pt = piece_to_piecetype(p);
    Colour piece_colour = piece_to_colour(p);


    while(piecebb) {
        Square s = pop_bit(piecebb);

        if(piece_colour == White) {
            weight += (piece_weights[pt] + (endgame ? white_eg_tables[pt][s] : white_mg_tables[pt][s])); 
        } else  {
            weight += (piece_weights[pt] + (endgame ? black_eg_tables[pt][s] : black_mg_tables[pt][s])); 
        }
    }

    weight += op_king_distance_from_center(board);
    return weight;
}

int eg_eval(const BoardState &board)
{
    int score = 0;
    for(Piece p = WQ; p <= WP; ++p) {
        score += eg_piece_weight(board, p);
    }

    for(Piece p = BQ; p <= BP; ++p) {
        score -= eg_piece_weight(board, p);
    }

    // Late endgame (King and rook, King and Queen, King and Bishop etc...
    if(board.get_num_piecetype(Pawn) == 0) {
        int dist_between_kings = king_distance(board);
        int wcount = relative_piece_count(board, White);
        int bcount = relative_piece_count(board, Black);
        int bking_distance_from_center = king_distance_from_center(board, Black) * 20;
        int wking_distance_from_center = king_distance_from_center(board, White) * 20;
        int king_distance_score = (7 - dist_between_kings) * 10;

        if(wcount > bcount) {
            score += bking_distance_from_center; 
            score += king_distance_score;
        } else if(wcount < bcount) {
            score -= wking_distance_from_center;
            score -= king_distance_score; 
        }
    }

    score += pawn_chain(board, White);
    score -= pawn_chain(board, Black);
    score += passed_pawn(board, White);
    score -= passed_pawn(board, Black);
    return score;
}

int mg_eval(const BoardState &board)
{
    int score = 0;
    for(Piece p = WQ; p <= WP; ++p) {
        score += mg_piece_weight(board, p);
    }

    for(Piece p = BQ; p <= BP; ++p) {
        score -= mg_piece_weight(board, p);
    }

    return score;
}

int eval(const BoardState &board)
{
    int mg = mg_eval(board);
    int eg = eg_eval(board);
    int phase = phase_weight(board);
    int score = ((mg * (256 - phase)) + (eg * phase)) / 256;
    return score;
}
