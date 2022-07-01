#include <climits>

#include "Generator.h"
#include "eval.h"
/* #include "BoardState.h" */
#include "search.h"

// FIXME: Hard to read, further refactors in order?

int Search::quiescence(BoardState board, int alpha, int beta, Line * pline, int max)
{
    int score = eval(board); 
    BMove captures[64];
    size_t num_captures = generate_captures(board, captures);

    return score;
}

int Search::alphabeta(
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

    // Just sort by captures for now
    std::sort(moves, moves + num_moves, [&](BMove a, BMove b) {
        uint8_t a_capture = (board.get_piece(get_to(a)) != None);
        uint8_t b_capture = (board.get_piece(get_to(b)) != None);

        return a_capture > b_capture;
    });

    int nega = us == White ? 1 : -1;
	if(depth == 0) {
        pline->num_moves = 0;
        // printf("Depth 0. Score: %d\n", eval(board) * nega);
        return eval(board) * nega;
    }

    // printf("Alpha before search: %d\n", alpha);
    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];

        board.make_move(m);
        if(!board.in_check(us)) {
            nodes_searched++;
            int score = -alphabeta(board, -beta, -alpha, depth - 1, &line); 

            if(score >= beta) 
                return beta; // fail hard

            if(score > alpha) {
                alpha = score;
                append_line(m);
            }
        }
        board.unmake_move(m);
    }

    // printf("Alpha after search: %d\n", alpha);

    return alpha;
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

BMove Search::iterative_search(BoardState board)
{
    size_t d = 0;
    std::vector<Line> lines(32); 
    std::vector<int> scores(32);
    BMove best_move;

    search_start = std::chrono::steady_clock::now();
    searching = true;

    while(searching) {
        scores[d] = search(board, d, &lines[d]);

        d++;
    }
    Line line;
    for(size_t j = 0; j < d - 1; ++j) {
        line = lines[j];
        // print_line(board, line);
        
        // std::cout << '\n';
    }
    std::cout << "Score: " << scores[d - 2] << '\n';
    return line.line[0];
}

int Search::search(BoardState board, size_t depth, Line * pline) 
{
    depth_searched = depth;
    Colour us = board.get_side_to_move(); 
    int inf = 999999;
    int alpha = us == White ? -inf : inf;
    int beta = us == White ? inf : -inf;
    return alphabeta(board, alpha, beta, depth, pline); 
}
