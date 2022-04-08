#include "perft.h"

int perft(int depth, BoardState board_state, Stats &stats)
{
   BMove moves[256];
   BoardState copy = board_state;
   int nodes = 0;
   int num_moves = 0;

   if(depth == 0)
       return 1ULL;

   num_moves = psuedo_generator(board_state, moves);

   for(int i=0; i<num_moves; ++i) {
       BMove m = moves[i];
       int flag = get_flag(m);
       int from = get_from(m);
       int to = get_to(m);

       Colour us = board_state.get_side_to_move();
       Piece p = board_state.get_piece(from);
       int kingsq = __builtin_ctzll(board_state.get_friend_piece_bb(King));
       bool check = (board_state.attacks_to(kingsq, us));
       bool capture = (board_state.get_piece(to) != None);

       board_state.make_move(m);
       if(!board_state.in_check(us)) {
           nodes += perft(depth - 1, board_state, stats);
           if(capture)
               stats.captures++;
           if(flag == OO || flag == OOO) 
               stats.castles++;
           if(flag == EN_PASSANT) {
               stats.ep++;
               stats.captures++;
           } 
           if(check) 
               stats.checks++;
       }
       board_state.unmake_move(m);

       if(!(copy == board_state)) {
           stats.bad_nodes++;
           if(capture)
               stats.bad_captures++;
           if(flag == EN_PASSANT)
               stats.bad_ep++;
           if(check)
               stats.bad_check++;
           if(flag == OO || flag == OOO) 
               stats.bad_castles++;
       }
   }
   return nodes;
}

void print_perft(int depth, BoardState board_state)
{
    Stats stats_arr[15];

    int nodes[15];

    for(int d = 1; d <= depth; ++d) {
        nodes[d] = perft(d, board_state, stats_arr[d]);
        Stats curr_stats = stats_arr[d];
        Stats prev_stats = stats_arr[d-1];
        curr_stats.nodes -= prev_stats.nodes;
        curr_stats.ep -= prev_stats.ep;
        curr_stats.captures -= prev_stats.captures;
        curr_stats.castles -= prev_stats.castles;

        std::cout 
            << "depth: " << d 
            << " | nodes: " << nodes[d] 
            << " | captures: " << curr_stats.captures 
            << " | EP: " << curr_stats.ep 
            << " | castles: " << curr_stats.castles 
            << " | checks: " << curr_stats.checks 
            << '\n';
    }
    std::cout
    << " ------------------------------------------------------------------------------ \n"
    << "make_move / unmake_move errors: \n";

    for(int d = 1; d <= depth; ++d) {
        nodes[d] = perft(d, board_state, stats_arr[d]);
        Stats curr_stats = stats_arr[d];
        std::cout
            << "depth: " << d
            << " | nodes: " << curr_stats.bad_nodes
            << " | captures: " << curr_stats.bad_captures
            << " | EP: " << curr_stats.bad_ep
            << " | check: " << curr_stats.bad_check
            << " | castles: " << curr_stats.bad_castles
            << '\n';
    }

}

bool test_in_check(BoardState board_state, Colour us) 
{
    return board_state.in_check(us);
}
