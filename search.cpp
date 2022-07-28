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
            }
        }
        board.unmake_move(m);
    }

    return alpha;
}

bool is_worth_looking_at(const TTItem * transposition, size_t current_depth) {
   return transposition != NULL && transposition->depth >= current_depth;
};

int Search::alphabeta(
        BoardState board,
        int alpha, 
        int beta, 
        size_t current_depth
) 
{
    nodes_searched++;

    if(!searching)
        return 0;
    
    Colour us = board.get_side_to_move();

    // Assume draw if position repeats so much as once for now
    if(board.is_repitition()) 
        return 0;
    
    if(current_depth == 0) 
        return quiescence(board, alpha, beta);

    int orig_alpha = alpha;
    ZobristKey key = board.get_hash();
    const TTItem * transposition = tt.get(key);
#if 1
    if(is_worth_looking_at(transposition, current_depth)) {
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
    }
#endif

    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);


#if 1
    std::sort(moves, moves + num_moves, [&](BMove a, BMove b) {

        ZobristKey azkey = board.generate_key_after_move(a);
        ZobristKey bzkey = board.generate_key_after_move(b);
        
        const TTItem * aitem = tt.get(azkey);
        const TTItem * bitem = tt.get(bzkey);

        bool a_worth = is_worth_looking_at(aitem, current_depth);
        bool b_worth = is_worth_looking_at(bitem, current_depth);

        return a_worth > b_worth;
    });
#endif
    // NOTE: Doing this sort before checking if depth 0 and doing quiescence causes invalid read segfault
    //       No clue as to why, look into later? Seems like the board object is being read but not allocated 

#if 1
    std::sort(moves, moves + num_moves, [&](BMove a, BMove b) {
         
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
#endif

    size_t num_legal_moves = 0;
    BMove nodes_best_move = moves[0];
    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        
        board.make_move(m);
        if(!board.in_check(us)) {
            num_legal_moves++;

            int score = -alphabeta(board, -beta, -alpha, current_depth - 1); 

            if(score >= beta) {
                // Score is too good, don't bother looking for more
                tt.insert(key, LOWER_BOUND, score, current_depth, m);
                return beta; // fail hard
            } 

            if(score > alpha) {
                alpha = score;
                nodes_best_move = m;
            } 
        }
        board.unmake_move(m);

    }

    if(nodes_best_move != 0) {
        // Alpha never raised 
        if(alpha <= orig_alpha)
            tt.insert(key, UPPER_BOUND, alpha, current_depth, nodes_best_move);
        else 
            tt.insert(key, EXACT, alpha, current_depth, nodes_best_move);
    }

    if(num_legal_moves == 0 && depth_searched > 1) {
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

void Search::print_info(BoardState board)
{
    std::string movestring;
#if 0
    pv.clear();
    pv.push_back(best_move);
    board.make_move(best_move);
    size_t d = 0;
    while(d < depth_searched) {
        ZobristKey key = board.get_hash();
        TTItem * item = tt.get(key);
        if(item != NULL && item->key == key) {
            BMove depth_best_move = item->move;
            pv.push_back(depth_best_move);
            board.make_move(depth_best_move);
        } 
        d++;
    }

    while(d > 0) {
        ZobristKey key = board.get_hash();
        TTItem * item = tt.get(key);
        if(item != NULL && item->key == key)
            board.unmake_move(item->move);
        d--;
    }

    for(size_t i = 0; i < pv.size(); ++i) {
        movestring += long_algebraic(pv[i]) + " ";
    }
#endif

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

    size_t inc = us == White ? winc : binc;
    size_t time = us == White ? wtime : btime;
    if(time) {
        movetime = 5000;
    }

    std::cout << "movetime " << movetime << " time " << time << '\n';

    search_start = std::chrono::steady_clock::now();
    searching = true;
    tt.reset();
    for(size_t d = 1; ; ++d) {

        ScoredMove sm = search(board, d);

        if(!searching)
            break;

        score = sm.score;
        best_move = sm.m;
        
        elapsed_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - search_start).count();

        if(elapsed_time > 0)
            d_times.push_back(elapsed_time);

        print_info(board); 

        if(sm.score == INF - 1) 
            break;

        // std::cout << "num transpositions " << tt.num_items << '\n';

        double predicted_time = static_cast<double>(elapsed_time) * 4;

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
    ZobristKey key = board.get_hash();

    BMove moves[256]; 
    size_t num_moves = psuedo_generator(board, moves);

    ScoredMove max_sm { moves[0], -INF };

    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];

        board.make_move(m);
        if(!board.in_check(us)) {
            int move_score = -alphabeta(board, -INF, INF, current_depth - 1);  

            if(move_score > max_sm.score) {
                max_sm.score = move_score;
                max_sm.m = m;
            }
        }
        board.unmake_move(m);
    }

    tt.insert(key, EXACT, max_sm.score, current_depth, best_move);

    return max_sm;
}
