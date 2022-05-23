#include <climits>

#include "Generator.h"
#include "eval.h"
#include "BoardState.h"
#include "search.h"

// NOTE: Tired, refactor later
// TODO: Count number of nodes searched, speed, etc...
int alphabeta_max(BoardState board, int alpha, int beta, int depth);
int alphabeta_min(BoardState board, int alpha, int beta, int depth);

int alphabeta_max(BoardState board, int alpha, int beta, int depth)
{
	if(depth == 0) return eval(board);

    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);
    Colour us = board.get_side_to_move();
    int score;

    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        board.make_move(m);
        if( !board.in_check(us) ) 
            score = alphabeta_min(board, alpha, beta, depth - 1); 
        board.unmake_move(m);
        if(score >= beta)
            return beta; // fail hard
        if(score > alpha)
            alpha = score;
    }
    return alpha;
}

int alphabeta_min(BoardState board, int alpha, int beta, int depth)
{
	if(depth == 0) return -1 * eval(board);

    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);
    Colour us = board.get_side_to_move();
    int score;

    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        board.make_move(m);
        if(!board.in_check(us)) 
            score = alphabeta_max(board, alpha, beta, depth - 1); 
        board.make_move(m);
        if(score <= alpha)
            return beta; // fail hard
        if(score < beta)
            beta = score;
    }
    return beta;
}

int search(BoardState board, int depth) 
{
    return alphabeta_max(board, INT_MIN, INT_MAX, depth);
}
