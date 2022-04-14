#include "Generator.h"

// Generate psuedo legal moves, return number of nodes 
int psuedo_generator(BoardState board_state, BMove moves[]) 
{   
    int i = 0; 
    Colour us = board_state.get_side_to_move();

    // temp?
    auto incr = [&](PieceType &pt) { pt = (PieceType)((int)pt + 1); };
    for(PieceType pt = Queen; pt <= Pawn; incr(pt)) {
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
                    moves[i] = move(origin, double_pawn_push, DOUBLE_PAWN_PUSH); 
                    i++;
                }

                // En Passant 
                Square epsq = board_state.get_ep_square();
                Square tosq = epsq + push_dir;
                bool pawn_adj = 
                     (bit(origin + W) & bit(epsq) & ~FileHBB) ||
                     (bit(origin + E) & bit(epsq) & ~FileABB);
                     
                if(epsq != None && pawn_adj) {
                    moves[i] = move(origin, tosq, EN_PASSANT); 
                    i++;
                }
            } 

            if(pt == King) {
                // Castles 
#if 0 
                Square kingsq = (us == White ? e1 : e8);
                if(board_state.can_castle(us, OOO)) {
                    Square to = (us == White ? d1 : d8); 
                    moves[i] = move(kingsq, to, OOO);
                    i++;
                }

                if(board_state.can_castle(us, OO)) {
                    Square to = (us == White ? g1 : g8); 
                    moves[i] = move(kingsq, to, OO);
                    i++;
                }
#endif 
            }

            // Regular attacks
            Bitboard to_squares = board_state.get_to_squares(pt, origin, us);
            while(to_squares) {
                Square dest = pop_bit(to_squares);
                moves[i] = move(origin, dest, QUIET); 
                i++;
            }
        }
    }
    return i;
}
