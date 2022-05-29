#include <climits>

#include "Generator.h"
#include "eval.h"
/* #include "BoardState.h" */
#include "search.h"

// NOTE: Tired, refactor later
// TODO: Count number of nodes searched, speed, etc...

int Search::alphabeta_max(
        BoardState board,
        int alpha, 
        int beta, 
        size_t depth 
) 
{
    auto now = std::chrono::steady_clock::now();
    Duration elapsed = now - search_start;

    if(elapsed.count() >= allotted) {
        searching = false;
        return eval(board);
    }

    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);
    Colour us = board.get_side_to_move();

	if(depth == 0) {
        return eval(board);
    }

    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        board.make_move(m);
        if(!board.in_check(us)) {
            int score = alphabeta_min(board, alpha, beta, depth - 1); 

            nodes_searched++;
            if(score >= beta)
                return beta; // fail hard
            if(score > alpha)
                alpha = score;
        }
        board.unmake_move(m);
    }
    return alpha;
}

int Search::alphabeta_min(
        BoardState board, 
        int alpha, 
        int beta, 
        size_t depth
) 
{
    auto now = std::chrono::steady_clock::now();
    Duration elapsed = now - search_start;

    if(elapsed.count() >= allotted) {
        searching = false;
        return eval(board);
    }

    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);
    Colour us = board.get_side_to_move();


	if(depth == 0) {
        return eval(board);
    }

    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        board.make_move(m);
        if(!board.in_check(us)) {
            int score = alphabeta_max(board, alpha, beta, depth - 1); 
            nodes_searched++;
            if(score <= alpha)
                return alpha; // fail hard
            if(score < beta)
                beta = score;
        }
        board.unmake_move(m);
    }
    return beta;
}

ScoredMove Search::best_move(BoardState board, size_t depth) 
{
    BMove moves[256];
    MoveList movelist;
    
    size_t num_moves = psuedo_generator(board, moves);
    Colour us = board.get_side_to_move();
    ScoredMove _best_move;
    
    if(us == White)
        _best_move = { 0, INT_MIN, "" };
    else _best_move = { 0, INT_MAX, "" }; 
    
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
            /* std::cout << i << ". { " << mi.algebraic << " : " << move_score << " } "; */
            if(
                (us == White && move_score > _best_move.score) || 
                (us == Black && move_score < _best_move.score)
            )
                _best_move = { m, move_score, mi.algebraic, board.get_movelist() };  
        }
        board.unmake_move(m);
    }
    return _best_move;
}

std::vector<ScoredMove> Search::iterative_search(BoardState board)
{
    size_t d = 0;
    std::vector<ScoredMove> best_move_at;
    search_start = std::chrono::steady_clock::now();
    searching = true;

    while(searching) {
        best_move_at.push_back(best_move(board, d));
        d++;
        depth_searched = d;
    }

    return best_move_at;
}

int Search::search(BoardState board, size_t depth) 
{
    return board.get_side_to_move() == White
        ? alphabeta_max(board, INT_MIN, INT_MAX, depth)
        : alphabeta_min(board, INT_MIN, INT_MAX, depth);
}
