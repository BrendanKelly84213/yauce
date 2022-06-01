#include <climits>

#include "Generator.h"
#include "eval.h"
/* #include "BoardState.h" */
#include "search.h"

// TODO: Store lines in MoveInfo or ScoredMove such that we can make iteratavely make moves and look at what the board looks like afterwords + further debugging


// NOTE: Not ideal... 
MoveInfo get_moveinfo(BMove m, BoardState board)
{
    Square from = get_from(m);
    Square to = get_to(m); 
    Piece p = board.get_piece(from);
    Piece cp = board.get_piece(to);

    PieceType pt = piece_to_piecetype(p);
    PieceType cpt = piece_to_piecetype(cp);

    Colour us = board.get_side_to_move();
    bool check = board.in_check(!us);

    MoveInfo mi(m, pt, cpt, us, check); 
    return mi;
}

std::string get_algstring(BMove m, BoardState board) 
{
    MoveInfo mi = get_moveinfo(m, board);
    return mi.algebraic;
}

int Search::alphabeta_max(
        BoardState board,
        int alpha, 
        int beta, 
        size_t depth,
        Line * pline
) 
{
    Line line;
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
        pline->num_moves = 0;
        return eval(board);
    }

    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        MoreMoveInfo chosen_move(m, board);
        board.make_move(m);
        if(!board.in_check(us)) {
            int score = alphabeta_min(board, alpha, beta, depth - 1, &line); 

            nodes_searched++;
            if(score >= beta) {
                return beta; // fail hard
            }

            if(score > alpha) {
                alpha = score;
                pline->line[0] = chosen_move;
                for(size_t d = 0; d < line.num_moves; ++d) 
                    pline->line[d + 1] = line.line[d];
                pline->num_moves = line.num_moves + 1;
            }
        }
        board.unmake_move(m);
    }

    return alpha;
}

int Search::alphabeta_min(
        BoardState board, 
        int alpha, 
        int beta, 
        size_t depth,
        Line * pline
) 
{
    Line line;
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
        pline->num_moves = 0;
        return eval(board);
    }

    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];

        MoreMoveInfo chosen_move(m, board); 

        board.make_move(m);
        if(!board.in_check(us)) {
            int score = alphabeta_max(board, alpha, beta, depth - 1, &line); 

            nodes_searched++;
            if(score <= alpha) {
                return alpha; // fail hard
            }
            if(score < beta) {
                beta = score;
                pline->line[0] = chosen_move;
                for(size_t d = 0; d < line.num_moves; ++d) 
                    pline->line[d + 1] = line.line[d];
                pline->num_moves = line.num_moves + 1;
            }
        }
        board.unmake_move(m);
    }

    return beta;
}

ScoredMove Search::best_move(BoardState board, size_t depth, Line * pline) 
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
        
        MoveInfo mi = get_moveinfo(m, board);
        board.make_move(m);
        if(!board.in_check(us)) {
            int move_score = search(board, depth, pline);

            if(
                (us == White && move_score > _best_move.score) || 
                (us == Black && move_score < _best_move.score)
            ) { 
                _best_move = { m, move_score, mi.algebraic, board.get_movelist() };  
            }
        }
        board.unmake_move(m);
    }
    return _best_move;
}

std::vector<ScoredMove> Search::iterative_search(BoardState board)
{
    size_t d = 0;
    std::vector<ScoredMove> best_move_at;
    // Lines at each depth level
    // Example: 
    //  lines[0] == "1. e4"
    //  lines[1] == "1. e4 2. e5"
    //  lines[2] == "1. e4, 2. e5, 3. Nf3"
    // Or something...
    std::vector<Line> lines(7); 
    /* Line * pline; */

    search_start = std::chrono::steady_clock::now();
    searching = true;

    while(searching) {
        depth_searched = d;
        ScoredMove bm = best_move(board, d, &lines[d]);
        best_move_at.push_back(bm);

        d++;
    }

    for(size_t j = 0; j < d; ++j) {
        Line line = lines[j];
        MoreMoveInfo m;
        for(size_t i = 0; i < 16; ++i) {
            m = line.line[i];
            
            std::cout << m.algebraic << " ";

        }
        std::cout << '\n';
    }

    return best_move_at;
}

int Search::search(BoardState board, size_t depth, Line * pline) 
{
    return board.get_side_to_move() == White
        ? alphabeta_max(board, INT_MIN, INT_MAX, depth, pline)
        : alphabeta_min(board, INT_MIN, INT_MAX, depth, pline);
}
