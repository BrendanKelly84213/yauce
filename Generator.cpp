#include "Generator.h"

// TODO: If in check:
//          only generate moves that move the king out of the way, attack the checkers or block the checkers if sliding
//
//       BoardState should store attacking checkers 
//           in_check should be cached value set on make_move?
//              Then we don't have to recompute attacking pieces 
//              This might be too much optimization this early on...

// Generate only moves required to get out of check
// TODO: Double checks
// NOTE: Putting this on the backburner for the next little while, since its too much optimization too soon...
//       The engine should be able to get the correct perft results without this anyway
int in_check_generator(BoardState board_state, BMove moves[]) 
{
    int i=0;
    Colour us = board_state.get_side_to_move();

    // King moves 
    Square king_sq = board_state.get_king_square(us);
    Bitboard king_to_squares = board_state.get_to_squares(King, king_sq, us);
    // NOTE: Potentially naive...
    while(king_to_squares) {
        Square to = pop_bit(king_to_squares);
        if(!board_state.attacked(to, !us)) {
            moves[i] = move(king_sq, to, QUIET);
            i++;
        }
    }

    // Counter attacks and blocks
    Bitboard checkers = board_state.checkers(us); // Attacks to the king
    while(checkers) {

        Square checking_from = pop_bit(checkers);

        // Blocking moves 
        // Get attacking piece
        Piece attacking_piece = board_state.get_piece(checking_from);
        PieceType attacking_piecetype = piece_to_piecetype(attacking_piece);
        const bool is_sliding = !(attacking_piecetype == Pawn || attacking_piecetype == Knight || attacking_piecetype == King);

        if(is_sliding) {

            // Squares between attacker and king
            Bitboard between_squares = get_sliding_until_ni(attacking_piecetype, checking_from, king_sq);
            while(between_squares) {

                Square blocker_to = pop_bit(between_squares);
                Bitboard blockers = board_state.attacks_to(blocker_to) & board_state.get_friend_occ(us);

                while(blockers) {
                    Square blocker_from = pop_bit(blockers);
                    moves[i] = move(blocker_from, blocker_to, QUIET);
                    i++;
                }
            }
        }

        // Counter attacks 
        Bitboard counter_attackers = board_state.attacks_to(checking_from) & board_state.get_friend_occ(us);
        while(counter_attackers) {

            Square counter_attacking_from = pop_bit(counter_attackers);
            // Move from the counter attackers square to the checkers square (capture)
            moves[i] = move(counter_attacking_from, checking_from, QUIET); // TODO: Come up with a better flag name than QUIET. 
            i++;
        }
    }

    return i;
}

int generate_captures(BoardState board, BMove captures[])
{
    int i = 0;
    Colour us = board.get_side_to_move(); 

    // Get squares attacking opponents pieces
    // Opponents piece bb
    Bitboard op_occ = board.get_op_occ();
    while(op_occ) {
        Square captured_square = pop_bit(op_occ);
        Bitboard attacks_to_captured_square = board.attacks_to(captured_square) & board.get_friend_occ();
        while(attacks_to_captured_square) {
            Square attacking_square = pop_bit(attacks_to_captured_square);

			// if(!(board.get_piece(captured_square) == WK || board.get_piece(captured_square) == BK)) {
			captures[i] = move(attacking_square, captured_square, QUIET);
			++i;
			// }
        }
    }
    return i;
}

// Generate psuedo legal moves, return number of nodes 
int psuedo_generator(BoardState board_state, BMove moves[]) 
{   
    int i = 0; 
    Colour us = board_state.get_side_to_move();
    const int second_to_last_rank = us == White ? 6 : 1;
    const int last_rank = us == White ? 7 : 0;

    for(PieceType pt = Queen; pt <= Pawn; ++pt) {

        Bitboard occ = board_state.get_friend_piece_bb(pt);

        while(occ) {
            Square origin = pop_bit(occ);

            // Special moves
            if(pt == Pawn) {
                // Double pawn push
                Direction push_dir = us == White ? N : S;  

                Square double_push = origin + push_dir + push_dir;
                Square single_push = origin + push_dir;

                bool fst_move =  
                         (us == White && rank(origin) == 1) ||
                         (us == Black && rank(origin) == 6);

                bool piece_on_single_push = (bit(single_push) & board_state.get_occ());
                bool piece_on_double_push = (bit(double_push) & board_state.get_occ());
                bool has_blockers = piece_on_single_push || piece_on_double_push;

                if(fst_move && !has_blockers) {
                    moves[i] = move(origin, double_push, DOUBLE_PAWN_PUSH); 
                    i++;
                }

                // En Passant 
                Square epsq = board_state.get_ep_square();
                Square tosq = epsq + push_dir;
                bool pawn_adj = 
                     (bit(origin + W) & bit(epsq) & ~FileHBB) ||
                     (bit(origin + E) & bit(epsq) & ~FileABB);
                     
                if(epsq != NullSquare  && pawn_adj) {
                    moves[i] = move(origin, tosq, EN_PASSANT); 
                    i++;
                }

                // Promotion
                if(rank(origin) == second_to_last_rank) {

                    const Move promotions[4] = { PROMOTE_QUEEN, PROMOTE_ROOK, PROMOTE_KNIGHT, PROMOTE_BISHOP };
                    Bitboard to_squares = board_state.get_to_squares(Pawn, origin, us);
                    while(to_squares) {

                        Square tosq = pop_bit(to_squares);
                        // For every available piece to promote to, add to movelist
                        for(size_t pi = 0; pi < 4; ++pi) {
                            moves[i] = move(origin, tosq, promotions[pi]); 
                            i++;
                        }
                    }
                }
            } 

            if(pt == King) {
                // Castles 
                Square kingsq = board_state.get_king_square(us);
                if(board_state.can_castle(us, OO)) {
                    Square to = (us == White ? g1 : g8); 
                    moves[i] = move(kingsq, to, OO);
                    i++;
                } 

                if(board_state.can_castle(us, OOO)) {
                    Square to = (us == White ? d1 : d8); 
                    moves[i] = move(kingsq, to, OOO);
                    i++;
                } 
            }


            // Regular attacks
            Bitboard to_squares = board_state.get_to_squares(pt, origin, us);

            while(to_squares) {
                Square dest = pop_bit(to_squares);
                if(!(pt == Pawn &&  dest == last_rank)) {
                    moves[i] = move(origin, dest, QUIET); 
                    i++;
                }
            }

        }
    }
    return i;
}
