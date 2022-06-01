#include <climits>

#include "Generator.h"
#include "eval.h"
/* #include "BoardState.h" */
#include "search.h"

// FIXME: Hard to read, further refactors in order?
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

    auto append_line = [&](BMove chosen_move) -> void {
        pline->line[0] = chosen_move;
        memcpy(pline->line + 1, line.line, line.num_moves * sizeof(BMove));
        pline->num_moves = line.num_moves + 1;
    };

    if(elapsed.count() >= allotted) {
        searching = false;
        return eval(board);
    }

    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);
    Colour us = board.get_side_to_move();

    std::sort(moves, moves + num_moves, [&](BMove a, BMove b) {
        bool a_capture = board.get_piece(get_to(a)) != None;
        bool b_capture = board.get_piece(get_to(b)) != None;
        return a_capture > b_capture;
    });

	if(depth == 0) {
        pline->num_moves = 0;
        return eval(board);
    }

    int value;
    if(max) {
        // Maximize
        for(size_t i = 0; i < num_moves; ++i) {
            BMove m = moves[i];

            board.make_move(m);
            if(!board.in_check(us)) {
                nodes_searched++;

                int score = alphabeta(board, alpha, beta, depth - 1, &line, false); // Minimize

                if(score >= beta) 
                    return beta; // fail hard

                if(score > alpha) {
                    alpha = score;
                    append_line(m);
                }
            }
            board.unmake_move(m);
        }
        value = alpha;
    } else {
        // Minimize
        for(size_t i = 0; i < num_moves; ++i) {
            BMove m = moves[i];

            board.make_move(m);
            if(!board.in_check(us)) {
                nodes_searched++;

                int score = alphabeta(board, alpha, beta, depth - 1, &line, true); // Maximize

                if(score <= alpha) 
                    return alpha; // fail hard
                
                if(score < beta) {
                    beta = score;
                    append_line(m);
                }
            }
            board.unmake_move(m);
        }
        value = beta;
    }
    return value;
}

void Search::print_line(BoardState board, Line line)
{
    BoardState copy = board;
    for(size_t i = 0; i < line.num_moves; ++i) {
        BMove m = line.line[i];
        std::cout << copy.get_algebraic(m) << " ";
        copy.make_move(m);
    }
    std::cout << '\n';
    copy.print_squares();
}

void Search::iterative_search(BoardState board)
{
    size_t d = 0;
    std::vector<Line> lines(7); 
    std::vector<int> scores(7);

    search_start = std::chrono::steady_clock::now();
    searching = true;

    while(searching) {
        scores[d] = search(board, d, &lines[d]);

        d++;
    }

    for(size_t j = 0; j < d - 1; ++j) {
        Line line = lines[j];
        print_line(board, line);
        
        std::cout << "Score: " << scores[j] << '\n';
        std::cout << '\n';
    }

}

int Search::search(BoardState board, size_t depth, Line * pline) 
{
    depth_searched = depth;
    return board.get_side_to_move() == White
        ? alphabeta(board, INT_MIN, INT_MAX, depth, pline, true)
        : alphabeta(board, INT_MIN, INT_MAX, depth, pline, false);
}
