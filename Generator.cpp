#include "Generator.h"

// TODO: If in check:
//          only generate moves that move the king out of the way, attack the checkers or block the checkers if sliding
//
//       BoardState should store attacking checkers 
//           in_check should be cached value set on make_move?
//              Then we don't have to recompute attacking pieces 
//              This might be too much optimization this early on...

int generate_captures(const BoardState &board, BMove captures[])
{
    int i = 0;
    Colour us = board.get_side_to_move(); 

    // Get squares attacking opponents pieces
    // Opponents piece bb
    Bitboard op_occ = board.get_op_occ(us);

    while(op_occ) {
        Square captured_square = pop_bit(op_occ);
        Bitboard attacks_to_captured_square = board.attacks_to(captured_square) & board.get_friend_occ(us);

        while(attacks_to_captured_square) {
            Square attacking_square = pop_bit(attacks_to_captured_square);

			captures[i] = move(attacking_square, captured_square, QUIET);
			++i;
        }
    }
    return i;
}

// Generate psuedo legal moves, return number of nodes 
int psuedo_generator(const BoardState & board_state, BMove moves[]) 
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
                    Square to = (us == White ? c1 : c8); 
                    moves[i] = move(kingsq, to, OOO);
                    i++;
                } 
            }

            // Regular attacks
            Bitboard to_squares = board_state.get_to_squares(pt, origin, us);

            while(to_squares) {
                Square dest = pop_bit(to_squares);
                if(!(pt == Pawn && dest == last_rank)) {
                    moves[i] = move(origin, dest, QUIET); 
                    i++;
                }
            }
        }
    }
    return i;
}
