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

int Search::alphabeta(
        BoardState board,
        int alpha, 
        int beta, 
        size_t depth,
        Line * pline,
        bool max
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

    int value;
    if(max) {
        // Maximize
        for(size_t i = 0; i < num_moves; ++i) {
            BMove m = moves[i];
            MoreMoveInfo chosen_move(m, board);

            board.make_move(m);
            if(!board.in_check(us)) {
                nodes_searched++;

                int score = alphabeta(board, alpha, beta, depth - 1, &line, false); // Minimize

                if(score >= beta) 
                    return beta; // fail hard

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
        value = alpha;
    } else {
        for(size_t i = 0; i < num_moves; ++i) {
            BMove m = moves[i];
            MoreMoveInfo chosen_move(m, board);

            board.make_move(m);
            if(!board.in_check(us)) {
                nodes_searched++;

                int score = alphabeta(board, alpha, beta, depth - 1, &line, true); // Maximize

                if(score <= alpha) 
                    return alpha; // fail hard
                
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
        value = beta;
    }
    return value;
}

void Search::iterative_search(BoardState board)
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
    std::vector<int> scores(7);

    search_start = std::chrono::steady_clock::now();
    searching = true;

    while(searching) {
        depth_searched = d;
        scores[d] = search(board, d, &lines[d]);

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
        std::cout << "Score: " << scores[j] << '\n';
    }
}

int Search::search(BoardState board, size_t depth, Line * pline) 
{
    return board.get_side_to_move() == White
        ? alphabeta(board, INT_MIN, INT_MAX, depth, pline, true)
        : alphabeta(board, INT_MIN, INT_MAX, depth, pline, false);
}
