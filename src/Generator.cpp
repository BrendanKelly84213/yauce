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

    for(int p = Queen; p <= Pawn; ++p) {
        Bitboard occ = board_state.get_friend_piece_bb(p);
        while(occ) {
            Square origin = pop_bit(occ);
            Move flag = QUIET; // Special move nibble 
            Square dest;
            moves[i] = 0;
            current_move = 0;

            // Special moves
            if(p == Pawn) {
                // Double pawn push
                Direction push_dir = us == White ? N : S;  
                Square double_push = origin + push_dir + push_dir;
                Square single_push = origin + push_dir;
                if((us == Black && origin >= a7 && origin <= h7) 
                || (us == White && origin >= a2 && origin <= h2)) {
                    if(!(bit(double_push) & board_state.get_op_occ()) && 
                        !(bit(single_push) & board_state.get_friend_occ())) {
                        moves[i] = ((origin << 10) | (double_push << 4));
                        moves[i] |= DOUBLE_PAWN_PUSH;
                        i++;
                    } 
                }

                // En Passant 
                Square epsq = board_state.get_ep_square();
                Square tosq = epsq + push_dir;
                     
                if(
                    epsq != None &&
                    (
                     (bit(origin + W) & bit(epsq) & ~FileHBB) ||
                     (bit(origin + E) & bit(epsq) & ~FileABB)
                    )
                ) {
                    moves[i] = ((origin << 10) | (tosq << 4));
                    moves[i] |= EN_PASSANT;
                    i++;
                }
            } 

            // Regular attacks
            Bitboard to_squares = board_state.get_to_squares(p, origin, us);
            while(to_squares) {
                dest = pop_bit(to_squares);
                moves[i] = ((origin << 10) | (dest << 4));
                i++;
            }
        }
    }
    return i;
}
