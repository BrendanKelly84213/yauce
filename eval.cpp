#include "eval.h"

const unsigned int P_weight = 100;
const unsigned int N_weight = 320;
const unsigned int B_weight = 330;
const unsigned int R_weight = 500;
const unsigned int Q_weight = 900;
const unsigned int K_weight = 20000;


// TODO: Balance tables.. Right now a knight in the center is worth more than a queen

// Piece tables 

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

int b_pawn_table[64];
int b_knight_table[64]; 
int b_bishop_table[64];
int b_rook_table[64];
int b_queen_table[64];

void init_black_tables()
{
    for(Square s = a1; s <= h8; s = s + E) {
        int curr_row = s >> 3;
        int offset = s % 8;
        int swap_row = 7 - curr_row;
        int swap_idx = offset + 8*swap_row;

        b_pawn_table[s] = w_pawn_table[swap_idx];
        b_knight_table[s] = w_knight_table[swap_idx];
        b_bishop_table[s] = w_bishop_table[swap_idx];
        b_rook_table[s] = w_rook_table[swap_idx];
        b_queen_table[s] = w_queen_table[swap_idx];
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

// TODO: King 

int piece_weight(BoardState board, Piece p)
{
    int weight = 0;
    Bitboard piecebb = board.get_piece_bb(p);
    while(piecebb) {
        Square s = pop_bit(piecebb);
        switch(p) {
            case WP : 
                weight += (P_weight + w_pawn_table[s]);
                break;
            case WB : 
                weight += (B_weight + w_bishop_table[s]);
                break;
            case WR : 
                weight += (R_weight + w_rook_table[s]);
                break;
            case WQ : 
                weight += (Q_weight + w_queen_table[s]);
                break;
            case WN : 
                weight += (N_weight + w_knight_table[s]);
                break;
            case BP : 
                weight += (P_weight + b_pawn_table[s]);
                break;
            case BB : 
                weight += (B_weight + b_bishop_table[s]);
                break;
            case BR : 
                weight += (R_weight + b_rook_table[s]);
                break;
            case BQ : 
                weight += (Q_weight + b_queen_table[s]);
                break;
            case BN : 
                weight += (N_weight + b_knight_table[s]);
                break;
            default : break;
        }
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
    );
}
