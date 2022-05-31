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
        Line & line
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

    MoveInfo chosen_move;
    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        chosen_move = get_moveinfo(m, board);
        board.make_move(m);
        if(!board.in_check(us)) {
            int score = alphabeta_min(board, alpha, beta, depth - 1, line); 

            nodes_searched++;
            if(score >= beta) {
                line[depth_searched - depth] = chosen_move;
                return beta; // fail hard
            }

            if(score > alpha)
                alpha = score;
        }
        board.unmake_move(m);
    }
    line[depth_searched - depth] = chosen_move;
    return alpha;
}

int Search::alphabeta_min(
        BoardState board, 
        int alpha, 
        int beta, 
        size_t depth,
        Line & line
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

    MoveInfo chosen_move;
    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];

        chosen_move = get_moveinfo(m, board);

        board.make_move(m);
        if(!board.in_check(us)) {
            int score = alphabeta_max(board, alpha, beta, depth - 1, line); 

            nodes_searched++;
            if(score <= alpha) {
                line[depth_searched - depth] = chosen_move;
                return alpha; // fail hard
            }
            if(score < beta) 
                beta = score;
        }
        board.unmake_move(m);
    }

    line[depth_searched - depth] = chosen_move;
    return beta;
}

ScoredMove Search::best_move(BoardState board, size_t depth, Line & line) 
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
            int move_score = search(board, depth, line);
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
    // Lines at each depth level
    // Example: 
    //  lines[0] == "1. e4"
    //  lines[1] == "1. e4 2. e5"
    //  lines[2] == "1. e4, 2. e5, 3. Nf3"
    // Or something...
    std::vector<Line> lines(7); 

    for(auto & l : lines) 
        l.resize(7);

    search_start = std::chrono::steady_clock::now();
    searching = true;

    while(searching) {
        /* best_move_at.push_back(best_move(board, d, lines[d])); */
        depth_searched = d;
        ScoredMove bm = best_move(board, d, lines[d]);
        best_move_at.push_back(bm);

        std::cout << bm.alg << " ";
        for(auto & m : lines[d]) {
            std::cout << m.algebraic << " " << ( m.m != 0 ? square_to_str(get_from(m.m)) : "" ) << (m.m != 0 ? square_to_str(get_to(m.m)): "" );
            std::cout << " ";
        }
        std::cout << '\n';
        d++;
    }

    return best_move_at;
}

int Search::search(BoardState board, size_t depth, Line & line) 
{
    return board.get_side_to_move() == White
        ? alphabeta_max(board, INT_MIN, INT_MAX, depth, line)
        : alphabeta_min(board, INT_MIN, INT_MAX, depth, line);
}
