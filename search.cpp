#include <climits>

#include "Generator.h"
#include "eval.h"
/* #include "BoardState.h" */
#include "search.h"

// NOTE: Tired, refactor later
// TODO: Count number of nodes searched, speed, etc...
int alphabeta_max(BoardState board, int alpha, int beta, int depth);
int alphabeta_min(BoardState board, int alpha, int beta, int depth);

int alphabeta_max(BoardState board, int alpha, int beta, int depth)
{
    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);
    Colour us = board.get_side_to_move();

	if(depth == 0) return eval(board);

    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        board.make_move(m);
        if(!board.in_check(us)) {
            int score = alphabeta_min(board, alpha, beta, depth - 1); 
            if(score >= beta)
                return beta; // fail hard
            if(score > alpha)
                alpha = score;
        }
        board.unmake_move(m);
    }
    return alpha;
}

int alphabeta_min(BoardState board, int alpha, int beta, int depth)
{
    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);
    Colour us = board.get_side_to_move();

	if(depth == 0) return -1 * eval(board);

    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        board.make_move(m);
        if(!board.in_check(us)) {
            int score = alphabeta_max(board, alpha, beta, depth - 1); 
            if(score <= alpha)
                return alpha; // fail hard
            if(score < beta)
                beta = score;
        }
        board.unmake_move(m);
    }
    return beta;
}

MoveList sorted_moves(BoardState board, int depth)
{
    BMove moves[256];
    MoveList movelist;
    
    size_t num_moves = psuedo_generator(board, moves);
    Colour us = board.get_side_to_move();
    
    for(size_t i = 0; i < num_moves; ++i) {

        BMove m = moves[i];
        Square from = get_from(m);
        Square to = get_to(m); 
        Piece p = board.get_piece(from);
        Piece cp = board.get_piece(to);

        PieceType pt = piece_to_piecetype(p);
        PieceType cpt = piece_to_piecetype(cp);
        
        board.make_move(m);
        if(!board.in_check(us)) {
            int move_score = search(board, depth);
            MoveInfo mi(m, pt, cpt, us, board.in_check(!us), move_score);
            std::cout << " { " << mi.algebraic << " : " << move_score << " } ";
            movelist.add(m, pt, cpt, us, board.in_check(!us), move_score);
        }
        board.unmake_move(m);
    }

    movelist.sort_by_score(us);

    return movelist;
}

int search(BoardState board, int depth) 
{
    return alphabeta_max(board, INT_MIN, INT_MAX, depth);
}
