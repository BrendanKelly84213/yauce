#include <chrono>
#include <climits>
#include <iterator>
#include <utility>

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
    if(!searching)
        return 0;

    Colour us = board.get_side_to_move();
	int nega = us == White ? 1 : -1;
 	int stand_pat = eval(board) * nega;
    if( stand_pat >= beta ) 
        return beta;
    
    if( alpha < stand_pat )
        alpha = stand_pat;

    nodes_searched++;
    
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
        Piece bcp = board.get_piece(bto);

        Piece bp = board.get_piece(bfrom);
        Piece ap = board.get_piece(afrom);

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
            int score = -quiescence(board, -beta, -alpha, &line); 

            if(score >= beta) 
                return beta; // fail hard

            if(score > alpha) {
                alpha = score;
                // append_line(m, line, pline);
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
        size_t current_depth,
        Line * pline
) 
{
    if(!searching)
        return 0;

    if(board.is_repeat())
        return 0;

    if(current_depth == 0)
        return quiescence(board, alpha, beta, pline);

    nodes_searched++;

    Line line;

    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);
    Colour us = board.get_side_to_move();

    // NOTE: Doing this sort before checking if depth 0 and doing quiescence causes invalid read segfault
    //       No clue as to why, look into later? Seems like the board object is being read but not allocated 
    std::sort(std::begin(moves), std::begin(moves) + num_moves, [board](BMove a, BMove b) {
        Square ato = get_to(a);
        Square bto = get_to(b);

        Piece acp = board.get_piece(ato);
        Piece bcp = board.get_piece(bto);
        int a_captured_weight = piece_weight(piece_to_piecetype(acp));
        int b_captured_weight = piece_weight(piece_to_piecetype(bcp));
        
        // Both are captures
        if(acp != None && bcp != None) {
            Square bfrom = get_from(b);
            Square afrom = get_from(a);
            Piece bp = board.get_piece(bfrom);
            Piece ap = board.get_piece(afrom);

            int a_capturing_weight = piece_weight(piece_to_piecetype(ap));

            int b_capturing_weight = piece_weight(piece_to_piecetype(bp));

            int a_diff = a_captured_weight - a_capturing_weight; 
            int b_diff = b_captured_weight - b_capturing_weight;  

            return a_diff > b_diff;
        }
        
        // Only one is a capture
        return a_captured_weight > b_captured_weight;
    });


    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        
        if(m == 0) {
            printf("Null Move (m == 0) \n");
            return 0;
        }

        board.make_move(m);
        if(!board.in_check(us)) {
            int score = -alphabeta(board, -beta, -alpha, current_depth - 1, &line); 

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

void Search::print_info(Line pv)
{
    std::string movestring;
    for(size_t i = 0; i < pv.num_moves; ++i) {
        movestring += long_algebraic(pv.line[i]) + " ";
    }

    elapsed_time++;
    std::cout << "info depth " << depth_searched; 
    std::cout << " nodes " << nodes_searched;
    std::cout << " score cp " <<  score;
    std::cout << " time " << elapsed_time;
    std::cout << " nps " << ( 1000 * ( nodes_searched / elapsed_time));
    std::cout << " pv " << movestring << std::endl;
}

void Search::iterative_search(BoardState board)
{
    Colour us = board.get_side_to_move();
    if(wtime && btime) {
        if(wtime >= 60000 * 5 || btime >= 60000 * 5) 
            movetime = 15000; 
        else movetime = 1000;
    }

    search_start = std::chrono::steady_clock::now();
    searching = true;
    for(size_t d = 1; ; ++d) {
        Line pv;
        score = search(board, d, &pv);

        if(!searching)
            break;

        best_move = pv.line[0];
        
        elapsed_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - search_start).count();

        if(elapsed_time > 0)
            d_times.push_back(elapsed_time);
        print_info(pv); 

        double predicted_time = static_cast<double>(elapsed_time) / 2;
      
        // Try to fit a y = ax^b curve to the predicted times 
        if(d > 3)  {
            double a, b;
            fit_power(a, b, d_times);
            predicted_time = a * pow(d + 1, b);
            // std::cout << "predicting " << predicted_time << "ms" << " a " << a << " b " << b << " ";
            // for(size_t i = 0; i < d_times.size(); ++i) {
            //     std::cout << d_times[i] << " ";
            // }
            // std::cout << '\n';
        } 

        bool depth_reached = depth && d >= depth;
        bool movetime_reached = movetime && predicted_time >= movetime;
        bool nodes_reached = nodes && nodes_searched >= nodes;

        if(!infinite && (depth_reached || movetime_reached || nodes_reached))
            break;
    }
    
    std::cout << "bestmove " << long_algebraic(best_move) << std::endl;  
}

int Search::search(BoardState board, size_t current_depth, Line * pline) 
{
    depth_searched = current_depth;
    return alphabeta(board, -999999, 999999, current_depth, pline); 
}
