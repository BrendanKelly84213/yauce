#include <climits>

#include "Generator.h"
#include "eval.h"
/* #include "BoardState.h" */
#include "search.h"

static inline void append_line(BMove chosen_move, Line line, Line * pline)
{
    pline->line[0] = chosen_move;
    memcpy(pline->line + 1, line.line, line.num_moves * sizeof(BMove));
    pline->num_moves = line.num_moves + 1;
}

int Search::quiescence(BoardState board, int alpha, int beta, Line * pline)
{
    // printf("In quiescence\n");
    Colour us = board.get_side_to_move();
	int nega = us == White ? 1 : -1;
 	int stand_pat = eval(board) * nega;
    if( stand_pat >= beta )
        return beta;
    if( alpha < stand_pat )
        alpha = stand_pat;
    
    Line line;
    BMove captures[256];
    size_t num_captures = generate_captures(board, captures);

    // Just sort by captures for now
    std::sort(captures, captures + num_captures, [&](BMove a, BMove b) {
        Square ato = get_to(a);
        Square afrom = get_from(a);
        Square bto = get_to(b);
        Square bfrom = get_from(b);

        Piece acp = board.get_piece(ato);
        Piece ap = board.get_piece(afrom);
        Piece bcp = board.get_piece(bto);
        Piece bp = board.get_piece(bfrom);

		int a_capturing_weight = piece_weight(piece_to_piecetype(ap));
		int a_captured_weight = piece_weight(piece_to_piecetype(acp));

		int b_capturing_weight = piece_weight(piece_to_piecetype(bp));
		int b_captured_weight = piece_weight(piece_to_piecetype(bcp));

        int a_diff = a_captured_weight - a_capturing_weight; 
        int b_diff = b_captured_weight - b_capturing_weight;  

        return a_diff > b_diff;
    });

    for(size_t i = 0; i < num_captures; ++i) {
        BMove m = captures[i];

        board.make_move(m);

        if(!board.in_check(us)) {
            // nodes_searched++;
            int score = -quiescence(board, -beta, -alpha, &line); 

            if(score >= beta) 
                return beta; // fail hard

            if(score > alpha) {
                alpha = score;
                append_line(m, line, pline);
            }
        }
        board.unmake_move(m);
    }

    return alpha;
}

int Search::alphabeta(
        BoardState board,
        int alpha, 
        int beta, 
        size_t depth,
        Line * pline
) 
{

    // auto now = std::chrono::steady_clock::now();
    // Duration elapsed = now - search_start;

    // if(elapsed.count() >= allotted) {
    //     searching = false;
    //     return eval(board);
    // }
    
	if(depth == 0) {
        return quiescence(board, alpha, beta, pline);
    }

    Line line;

    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);
    Colour us = board.get_side_to_move();

    // NOTE: Doing this sort before checking if depth 0 and doing quiescence causes invalid read segfault
    //       No clue as to why, look into later? Seems like the board object is being read
#if 1
    std::sort(std::begin(moves), std::end(moves), [board](BMove a, BMove b) {


        const Square afrom = get_from(a);
        const Square ato = get_to(a);

        const Square bfrom = get_from(b);
        const Square bto = get_to(b);

        const Piece ap = board.get_piece(afrom);
        const Piece acp = board.get_piece(ato);

        const Piece bp = board.get_piece(bfrom);
        const Piece bcp = board.get_piece(bto);

        const bool a_capture = (acp!= None);
        const bool b_capture = (bcp!= None);

		const int a_capturing_weight = piece_weight(piece_to_piecetype(ap));
		const int a_captured_weight = piece_weight(piece_to_piecetype(acp));

		const int b_capturing_weight = piece_weight(piece_to_piecetype(bp));
		const int b_captured_weight = piece_weight(piece_to_piecetype(bcp));

        const int a_diff = a_captured_weight - a_capturing_weight; 
        const int b_diff = b_captured_weight - b_capturing_weight;  

        return (a_capture > b_capture) || (a_diff > b_diff);
    });
#endif

    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        
        if(m == 0) {
            printf("Null Move (m == 0) \n");
            return 0;
        }

        board.make_move(m);
        if(!board.in_check(us)) {
            int score = -alphabeta(board, -beta, -alpha, depth - 1, &line); 

            if(score >= beta) 
                return beta; // fail hard

            if(score > alpha) {
                alpha = score;
                append_line(m, line, pline);
            }
        }
        board.unmake_move(m);
    }

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
    return alphabeta(board, -999999, 999999, depth, pline); 
}
