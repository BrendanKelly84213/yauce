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
       int flag = m & 0xf;
       int from = (m >> 10) & 0x3f;
       int to = (m >> 4) & 0x3f;

       Colour us = board_state.state.side_to_move;
       Piece p = board_state.squares[from];
       board_state.make_move(m);
       if(!board_state.in_check(us)) {
           nodes += perft(depth - 1, board_state, stats);
           if(flag == CAPTURE)
               stats.captures++;
           else if(flag == OO || flag == OOO) 
               stats.castles++;
           else if(flag == EN_PASSANT) {
               stats.ep++;
               stats.captures++;
           } else if(flag == CHECK) {
               stats.checks++;
           }
       }
       board_state.unmake_move(m);

       if(!(copy == board_state)) {
           if(flag == CAPTURE)
               stats.bad_captures++;
           else if(flag == EN_PASSANT)
               stats.bad_ep++;
           else if(flag == CHECK)
               stats.bad_check++;
           else if(flag == OO || flag == OOO) 
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
