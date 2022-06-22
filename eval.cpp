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

void init_black_tables()
{
    for(Square s = a1; s <= h8; s = s + E) {
        int curr_row = s >> 3;
        int offset = s % 8;
        int swap_row = 7 - curr_row;
        int swap_idx = offset + 8*swap_row;

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

int piece_weight(BoardState board, Piece p)
{
    int weight = 0;
    bool endgame = board.in_endgame();
    Bitboard piecebb = board.get_piece_bb(p);
    PieceType pt = piece_to_piecetype(p);
    Colour piece_colour = piece_to_colour(p);

    while(piecebb) {
        Square s = pop_bit(piecebb);

        if(piece_colour == White) 
            weight += piece_weights[pt] + (endgame ? white_eg_tables[pt][s] : white_mg_tables[pt][s]); 
        else 
            weight += piece_weights[pt] + (endgame ? black_eg_tables[pt][s] : black_mg_tables[pt][s]); 
    }
    return weight;
}

// Just return the material inbalance for now
// NOTE: I am very tired and don't really gaf 
int eval(BoardState board)
{
    return (
     (piece_weight(board, WP) - piece_weight(board, BP))
    + (piece_weight(board, WB) - piece_weight(board, BB))
    + (piece_weight(board, WR) - piece_weight(board, BR))
    + (piece_weight(board, WQ) - piece_weight(board, BQ))
    + (piece_weight(board, WN) - piece_weight(board, BN))
    + (piece_weight(board, WK) - piece_weight(board, BK))
    );
}
