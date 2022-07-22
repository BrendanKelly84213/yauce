#include <chrono>
#include <climits>
#include <iterator>
#include <utility>
#include <algorithm>

#include "Generator.h"
#include "eval.h"
/* #include "BoardState.h" */
#include "search.h"

const int INF = 999999;

int Search::quiescence(BoardState board, int alpha, int beta)
{
    if(!searching)
        return 0;
    
    Colour us = board.get_side_to_move();
    int nega = us == White ? 1 : -1;
 	int stand_pat = eval(board) * nega;

    if(stand_pat >= beta) 
        return beta;
    if(stand_pat > alpha) 
        alpha = stand_pat;

    nodes_searched++;
    
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

    size_t num_legal_moves = 0;
    for(size_t i = 0; i < num_captures; ++i) {
        BMove m = captures[i];

        board.make_move(m);

        if(!board.in_check(us)) {
            num_legal_moves++;
            int score = -quiescence(board, -beta, -alpha); 

            if(score >= beta) 
                return beta; // fail hard

            if(score > alpha) {
                alpha = score;
                if(alpha == (INF - 1))
                    break;
            }
        }
        board.unmake_move(m);
    }

    if(num_legal_moves == 0 && num_captures > 0) {
        // Checkmate
        if(board.in_check(us)) 
            return -INF + 1; 
        // Stalemate
        return 0;
    }

    return alpha;
}

int Search::alphabeta(
        BoardState board,
        int alpha, 
        int beta, 
        size_t current_depth,
        Line &line
) 
{
    if(!searching)
        return 0;

    Colour us = board.get_side_to_move();

    Line node_line(depth_searched); 

    // Assume draw if position repeats so much as once for now
    if(board.is_repitition()) 
        return 0;
    
    if(current_depth == 0) 
        return quiescence(board, alpha, beta);

    nodes_searched++;

    TTItem * transposition = tt.get(board.get_hash());
    if(transposition != NULL && transposition->depth >= current_depth) {
        int score = transposition->score;
        switch(transposition->type) {
            case EXACT: 
                return score;
            case UPPER_BOUND: 
                beta = std::min(beta, score);
                break;
            case LOWER_BOUND: 
                alpha = std::max(alpha, score);
                break;
            default: break;          
        }
        // if(alpha >= beta)
        //     return score;
    }
    
    int orig_alpha = alpha;

    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);

    // NOTE: Doing this sort before checking if depth 0 and doing quiescence causes invalid read segfault
    //       No clue as to why, look into later? Seems like the board object is being read but not allocated 
    // NOTE: Only take captures into account for now
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

    size_t num_legal_moves = 0;
    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        
        board.make_move(m);
        if(!board.in_check(us)) {
            num_legal_moves++;

            int score = -alphabeta(board, -beta, -alpha, current_depth - 1, node_line); 

            if(score >= beta) {
                tt.insert(board.get_hash(), LOWER_BOUND, score, current_depth);
                return beta; // fail hard
            } 

            if(score > alpha) {
                alpha = score;
                line[0] = m;
                std::copy(node_line.begin(), node_line.end(), line.begin() + 1);
                if(alpha == (INF - 1)) 
                    break;
            }
        }
        board.unmake_move(m);
    }

    if(alpha <= orig_alpha) 
        tt.insert(board.get_hash(), UPPER_BOUND, alpha, current_depth);
    else 
        tt.insert(board.get_hash(), EXACT, alpha, current_depth);

    if(num_legal_moves == 0) {
        // Checkmate
        if(board.in_check(us)) 
            return -INF + 1; 
        
        // Stalemate
        return 0;
    }

    return alpha;
}

void Search::print_pv(Line line)
{
    for(size_t i = 0; i < line.size(); ++i) {
        BMove m = line[i];
        std::cout << long_algebraic(m) << " ";
    }
    std::cout << '\n';
}

void Search::print_info()
{
    std::string movestring;
    for(size_t i = 0; i < pv.size(); ++i) {
        movestring += long_algebraic(pv[i]) + " ";
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
        size_t inc = us == White ? winc : binc;
        size_t time = us == White ? wtime : btime;
        if(time >= 60 * 1000) {
            if(!inc)
                movetime = 1000;
            else movetime = 1000 + inc;
        } else movetime = 500;
       
    }

    search_start = std::chrono::steady_clock::now();
    searching = true;
    for(size_t d = 1; ; ++d) {

        ScoredMove sm = search(board, d);

        if(!searching)
            break;

        score = sm.score;
        best_move = sm.m;
        pv = sm.line;
        
        elapsed_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - search_start).count();

        if(elapsed_time > 0)
            d_times.push_back(elapsed_time);

        print_info(); 

        double predicted_time = static_cast<double>(elapsed_time) / 2;
      
        // Try to fit a y = ax^b curve to the predicted times 
        if(d > 3)  {
            double a, b;
            fit_power(a, b, d_times);
            predicted_time = a * pow(d + 1, b);
        } 

        bool depth_reached = depth && d >= depth;
        bool movetime_reached = movetime && (predicted_time >= movetime);
        bool nodes_reached = nodes && nodes_searched >= nodes;

        if(!infinite && (depth_reached || movetime_reached || nodes_reached))
            break;
    }
    
    std::cout << "bestmove " << long_algebraic(best_move) << std::endl;  
}

ScoredMove Search::search(BoardState board, size_t current_depth) 
{
    Colour us = board.get_side_to_move(); 
    depth_searched = current_depth;

    BMove moves[256]; 
    size_t num_moves = psuedo_generator(board, moves);

    ScoredMove max_sm = { move(a1, a1, QUIET), -INF, Line(depth_searched) };

    Line line(depth_searched);
    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];

        board.make_move(m);
        if(!board.in_check(us)) {
            int move_score = -alphabeta(board, -INF, INF, current_depth - 1, line);  

            if(move_score > max_sm.score) {
                max_sm.score = move_score;
                max_sm.m = m;
                max_sm.line[0] = m;
                std::copy(line.begin(), line.end(), max_sm.line.begin() + 1);
            }
        }
        board.unmake_move(m);
    }

    return max_sm;
}
