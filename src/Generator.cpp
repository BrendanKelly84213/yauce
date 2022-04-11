#include "Generator.h"

// Generate psuedo legal moves, return number of nodes 
int psuedo_generator(BoardState board_state, BMove moves[]) 
{   
    BMove current_move = 0;
    int i = 0; 
    Colour us = board_state.get_side_to_move();
    Square kingsq = (us == White ? e1 : e8);

    // Castle 
#if 0
    if(board_state.can_castle(us, OOO)) {
        int to = (us == White ? d1 : d8); 
        moves[i] = ((kingsq << 10) | (to << 4));
        moves[i] |= OOO;
        i++;
    }
    if(board_state.can_castle(us, OO)) {
        int to = (us == White ? g1 : g8); 
        moves[i] = ((kingsq << 10) | (to << 4));
        moves[i] |= OO;
        i++;
    }
#endif

    auto incr = [&](PieceType &pt) { pt = (PieceType)((int)pt + 1); };
    for(PieceType pt = Queen; pt <= Pawn; incr(pt)) {
        Bitboard occ = board_state.get_friend_piece_bb(pt);
        while(occ) {
            Square origin = pop_bit(occ);
            Move flag = QUIET; // Special move nibble 
            Square dest;
            moves[i] = 0;
            current_move = 0;

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
                    // FIXME: implement set_move
                    moves[i] = ((origin << 10) | (double_push << 4));
                    moves[i] |= DOUBLE_PAWN_PUSH;
                    i++;
                }

                // En Passant 
                Square epsq = board_state.get_ep_square();
                Square tosq = epsq + push_dir;
                bool pawn_adj = 
                     (bit(origin + W) & bit(epsq) & ~FileHBB) ||
                     (bit(origin + E) & bit(epsq) & ~FileABB);
                     
                if(epsq != None && pawn_adj) {
                    // FIXME: implement set_move
                    moves[i] = ((origin << 10) | (tosq << 4));
                    moves[i] |= EN_PASSANT;
                    i++;
                }
            } 

            // Regular attacks
            Bitboard to_squares = board_state.get_to_squares(pt, origin, us);
            while(to_squares) {
                dest = pop_bit(to_squares);
                // FIXME: implement set_move
                moves[i] = ((origin << 10) | (dest << 4));
                i++;
            }
        }
    }
    return i;
}
