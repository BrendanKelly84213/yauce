#include "Generator.h"

// Returns all attack squares of all pieces 
// probably could be refactored, only used in castling atm, very slow
Bitboard get_attack_squares(BoardState board_state, Colour us)
{
    Bitboard attack_squares = 0ULL; 
    for(int pt=0; pt<6; ++pt) {
        Bitboard piece_bb = board_state.get_side_piece_bb(pt, us);
        while(piece_bb) {
            Square occ_sq = pop_bit(piece_bb);
            attack_squares |= board_state.get_to_squares(pt, occ_sq, us);
        }
    }
    return attack_squares;
}

// ############################### CASTLING ############################### 
bool can_castle_ks(BoardState board_state) 
{
    Colour us = board_state.state.side_to_move; 
    bool rights;
    Square k_sq, k_adj, r_adj, r_sq; // King and rook squares and adjacent squares
    Colour op = board_state.state.side_to_move == White ? Black : White;
    Bitboard attack_squares = get_attack_squares(board_state, op);
    if(us == White) { 
        rights = board_state.state.w_castle_ks;
        k_sq  = e1;
        k_adj = f1;
        r_adj = g1;
        r_sq  = h1;
    } else {
        rights = board_state.state.b_castle_ks;
        k_sq  = e8;
        k_adj = f8;
        r_adj = g8;
        r_sq  = h8;
    }
    if(!rights) {
        return false;
    }
    if(us == White 
    && (board_state.get_piece(r_sq) != WR || board_state.get_piece(k_sq) != WK)) {
        return false;
    }
    if(us == Black 
    && (board_state.get_piece(r_sq) != BR || board_state.get_piece(k_sq) != BK)) {
        return false;
    }
    // Blockers 
    if(board_state.get_occ() & (bit(k_adj) | bit(r_adj))) {
        return false;
    }
    // Attackers
    if(attack_squares & (bit(k_sq) | bit(k_adj) | bit(r_adj) | bit(r_sq))) {
        return false;
    }
    return true;
}

bool can_castle_qs(BoardState board_state) 
{
    Colour us = board_state.state.side_to_move; 
    bool rights;
    Square k_sq, k_adj1, k_adj2, r_adj, r_sq; // King and rook squares and adjacent squares
    Colour op = board_state.state.side_to_move == White ? Black : White;
    Bitboard attack_squares = get_attack_squares(board_state, op);
    if(us == White) { 
        rights = board_state.state.w_castle_qs;
        k_sq   = e1;
        k_adj1 = d1;
        k_adj2 = c1;
        r_adj  = b1;
        r_sq   = a1;
    } else {
        rights = board_state.state.b_castle_qs;
        k_sq   = e8;
        k_adj1 = d8;
        k_adj2 = c8;
        r_adj  = b8;
        r_sq   = a8;
    }
    if(!rights) {
        return false;
    }
    if(us == White 
    && (board_state.get_piece(r_sq) != WR || board_state.get_piece(k_sq) != WK)) {
        return false;
    }
    if(us == Black 
    && (board_state.get_piece(r_sq) != BR || board_state.get_piece(k_sq) != BK)) {
        return false;
    }
    // Blockers 
    if(board_state.get_occ() & (bit(k_adj1) | bit(k_adj2) | bit(r_adj))) {
        return false;
    }
    // Attackers
    if(attack_squares & (bit(k_sq) | bit(k_adj1) | bit(k_adj2) | bit(r_adj) | bit(r_sq))) {
        return false;
    }
    return true;
}

// ############################################################## 

// Generate psuedo legal moves, return number of nodes 
int psuedo_generator(BoardState board_state, BMove moves[]) 
{   
    BMove current_move = 0;
    int i = 0; 
    Colour us = board_state.state.side_to_move;
    int kingsq = (us == White ? e1 : e8);

    if(can_castle_qs(board_state)) {
        int to = (us == White ? d1 : d8); 
        current_move = ((kingsq << 10) | (to << 4));
        current_move |= OOO;
        moves[i] = current_move;
        i++;
    }
    if(can_castle_ks(board_state)) {
        int to = (us == White ? g1 : g8); 
        current_move = ((kingsq << 10) | (to << 4));
        current_move |= OO;
        moves[i] = current_move;
        i++;
    }

    for(int p = Queen; p <= Pawn; ++p) {
        Bitboard occ = board_state.get_friend_piece_bb(p);
        while(occ) {
            BMove origin = (BMove)pop_bit(occ);
            Move flag = QUIET; // Special move nibble 
            BMove dest;
            moves[i] = 0;
            current_move = 0;
            Bitboard to_squares = board_state.get_to_squares(p, origin, us);
            while(to_squares) {
                dest = (BMove)pop_bit(to_squares);
                current_move = ((origin << 10) | (dest << 4));
                flag = QUIET;

                if(p == Pawn) {
                    if (bit(dest) == bit(origin + N + N) || bit(dest) == bit(origin + S + S)) 
                        flag = DOUBLE_PAWN_PUSH;
                    else if(
                            board_state.state.ep_file != -1
                        && !(bit(dest) & board_state.get_occ()) // No piece on dest 
                        && ((us == White && (origin >> 3) == 5) || (us == Black && (origin >> 3) == 4)) // origin on ep_rank
                        && (dest % 8 == board_state.state.ep_file) // dest is same file as ep pawn
                    )  
                        flag = EN_PASSANT;
                } 

                current_move |= flag;
                moves[i] = current_move;
                i++;
            }
        }
    }
    return i;
}
