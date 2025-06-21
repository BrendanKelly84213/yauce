#include <chrono>
#include <climits>
#include <iterator>
#include <utility>
#include <algorithm>

#include "Generator.h"
/* #include "BoardState.h" */
#include "search.h"

const int INF = 999999;

bool is_worth_looking_at(const TTItem * transposition, size_t current_depth) 
{
   return transposition != NULL && transposition->depth >= current_depth;
};

int Search::q_relative_value(BMove m, const BoardState & board) const 
{
    Square from = get_from(m);
    Square to = get_to(m);
    Piece p = board.get_piece(from);
    Piece cp = board.get_piece(to);
    PieceType apt = piece_to_piecetype(p);
    PieceType vpt = piece_to_piecetype(cp);
    PieceOrder v = ordering_table[vpt];
    PieceOrder a = ordering_table[apt];

    return mvvlva_table[v][a]; 
}

int Search::relative_value(BMove m, const BoardState & board, size_t current_depth) const 
{
    ZobristKey key = board.get_hash();
    const TTItem * transposition = tt.get(key); 

    Square from = get_from(m);
    Square to = get_to(m);
    Piece p = board.get_piece(from);
    Piece cp = board.get_piece(to);
    Move flag = get_flag(m);

    if(transposition && m == transposition->move) {
        if(transposition->type == EXACT) 
            return PV;
        return Hash;
    } else if(flag >= PROMOTE_QUEEN && flag <= PROMOTE_BISHOP) {
        return Promotion;
    } else if(cp != None) {
        PieceType apt = piece_to_piecetype(p);
        PieceType vpt = piece_to_piecetype(cp);
        PieceOrder v = ordering_table[vpt];
        PieceOrder a = ordering_table[apt];
        return mvvlva_table[v][a]; 
    } else if(m == killers[current_depth][0]) {
            return KillerMove1;
    } else if(m == killers[current_depth][1]) {
        return KillerMove2;
    } else {
        return NonCapture;
    } 
}

void Search::print_movelist_vals(BMove moves[], size_t num_moves, const BoardState & board, size_t current_depth) const
{
    for(size_t i = 0; i < num_moves; ++i) {
        std::cout << relative_value(moves[i], board, current_depth) << " ";
    }
    std::cout << '\n';
}

void print_movelist(BMove moves[], size_t num_moves)
{
    for(size_t i = 0; i < num_moves; ++i) {
        std::cout << long_algebraic(moves[i]) << " ";
    }
    std::cout << '\n';
}

int Search::quiescence(BoardState board, int alpha, int beta, size_t qdepth)
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

    BMove captures[256];
    size_t num_captures = generate_captures(board, captures);

    nodes_searched++;

    std::sort(captures, captures + num_captures, [&](BMove a, BMove b) {
       return q_relative_value(a, board) > q_relative_value(b, board); 
    });

    size_t num_legal_moves = 0;
    for(size_t i = 0; i < num_captures; ++i) {
        BMove m = captures[i];

        board.make_move(m);

        if(!board.in_check(us)) {
            num_legal_moves++;
            int score = -quiescence(board, -beta, -alpha, qdepth - 1); 
            
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
    
    if(current_depth == 0) {
        return quiescence(board, alpha, beta, 4);
    }

    int orig_alpha = alpha;
    ZobristKey key = board.get_hash();
    const TTItem * transposition = tt.get(key);
    if(is_worth_looking_at(transposition, current_depth)) {
        num_transpositions++;
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

        if(alpha >= beta)
            return score;
    }

    // Null move pruning
    if(!board.in_check(us) && current_depth > 2) {
        board.make_null_move();
        int score = -alphabeta(board, -beta, -beta + 1, current_depth - 1 - 2);
        board.unmake_null_move();
        if(score >= beta)
            return beta;
    }

    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);

    // NOTE: Doing this sort before checking if depth 0 and doing quiescence causes invalid read segfault
    //       No clue as to why, look into later? Seems like the board object is being read but not allocated 
    
    std::sort(moves, moves + num_moves, [&](BMove a, BMove b) {
       return relative_value(a, board, current_depth) > relative_value(b, board, current_depth); 
    });

    size_t num_legal_moves = 0;
    BMove nodes_best_move = moves[0];
    for(size_t i = 0; i < num_moves; ++i) {
        BMove m = moves[i];
        Square to = get_to(m);
        bool is_capture = (board.get_piece(to) != None);

        board.make_move(m);
        if(!board.in_check(us)) {
            num_legal_moves++;

            int score = -alphabeta(board, -beta, -alpha, current_depth - 1); 

            // Score is too good, don't bother looking for more
            if(score >= beta) {
                if(!is_capture) 
                    update_killers(m, current_depth);

                tt.insert(key, LOWER_BOUND, score, current_depth, m);

                return beta; 
            } 

            if(score > alpha) {
                alpha = score;
                nodes_best_move = m;
            } 
        }
        board.unmake_move(m);
    }

    if(num_legal_moves == 0) {
        // Checkmate
        if(board.in_check(us)) 
            return -INF + 1; 
        
        // Stalemate
        return 0;
    }

    // Alpha never raised 
    if(alpha <= orig_alpha)
        tt.insert(key, UPPER_BOUND, alpha, current_depth, nodes_best_move);
    else 
        tt.insert(key, EXACT, alpha, current_depth, nodes_best_move);

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
    pv.clear();
    pv.push_back(best_move);
    board.make_move(best_move);
    size_t d = 0;
    while(d < depth_searched) {
        ZobristKey key = board.get_hash();
        const TTItem * item = tt.get(key);
        if(item != NULL && item->key == key) {
            BMove depth_best_move = item->move;
            pv.push_back(depth_best_move);
            board.make_move(depth_best_move);
        } 
        d++;
    }

    while(d > 0) {
        ZobristKey key = board.get_hash();
        const TTItem * item = tt.get(key);
        if(item != NULL && item->key == key)
            board.unmake_move(item->move);
        d--;
    }

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

void Search::iterative_search(const BoardState& board)
{
    Colour us = board.get_side_to_move(); 

    size_t inc = us == White ? winc : binc;
    size_t time = us == White ? wtime : btime;
    if(movesleft > 2)
        movesleft--;
    else 
        movesleft = 10;
    if(time) {
        movetime = std::round(1.3 * time / (movesleft + 2.9)) + inc;
    }

    search_start = std::chrono::steady_clock::now();
    searching = true;
    // NOTE: resetting the transposition every search is a dumb hack workaround to an unknown bug (probably) in the transposition table
    //          (where if not reset, engine makes horrible moves at random, queen sacrifices, losing mate in 3, etc...
    //          coincides with very short pv's, possibly bug in transposition table).
    //       It doesn't have that big of an effect on depth searched per move in the middlegame, endgame seems to suffer more
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

        if(std::abs(sm.score) >= INF - 1) 
            break;

        double predicted_time = static_cast<double>(elapsed_time) * 3;

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
