#include "perft.h"

int perft(int depth, BoardState board_state, Stats &stats)
{
   BMove moves[256];
   /* BoardState copy = board_state; */
   int nodes = 0;
   int num_moves = 0;

   if(depth == 0)
       return 1ULL;

   Colour us = board_state.get_side_to_move();
   num_moves = psuedo_generator(board_state, moves);

   for(int i=0; i<num_moves; ++i) {
       BMove m = moves[i];
       Move flag = get_flag(m);
       Square from = get_from(m);
       Square to = get_to(m);

       Piece p = board_state.get_piece(from);

       bool capture = (board_state.get_piece(to) != None);

       board_state.make_move(m);
       if(!board_state.in_check(us)) {
           if(capture) {
               stats.captures++;
               /* board_state.print_context(m, capture, flag); */
           }
           if(flag == OO || flag == OOO) 
               stats.castles++;
           if(flag == EN_PASSANT) {
               stats.ep++;
               stats.captures++;
           } 
           if(board_state.in_check(!us)) {
               stats.checks++;
               
               // Calc checkmate (Naively but for testing its honestly fine for now)
               BMove opp_psuedo_legal_moves[256];
               BoardState copy_state = board_state;
               size_t num_opp_legal_moves = 0;
               const size_t num_opp_psuedo_legal_moves = psuedo_generator(board_state, opp_psuedo_legal_moves);
               for(size_t j = 0; j < num_opp_psuedo_legal_moves ; ++j) {
                   BMove m = opp_psuedo_legal_moves[j];
                   copy_state.make_move(m);
                   if(!copy_state.in_check(!us))
                       num_opp_legal_moves++;
                   copy_state.unmake_move(m);
               }

               if(num_opp_legal_moves == 0)
                   stats.checkmates++;
           }
           if(flag >= PROMOTE_QUEEN && flag <= PROMOTE_BISHOP)
               stats.promotions++;
           nodes += perft(depth - 1, board_state, stats);
       } 

       board_state.unmake_move(m);

#if 0
       if(!(copy == board_state)) {
           stats.bad_nodes++;
           if(capture)
               stats.bad_captures++;
           if(flag == EN_PASSANT)
               stats.bad_ep++;
           if(board_state.in_check(!us)) // I don't think this works... getting tired... 
               stats.bad_check++;
           if(flag == OO || flag == OOO) 
               stats.bad_castles++;
       }
#endif

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
        curr_stats.checks -= prev_stats.checks;
        curr_stats.checkmates -= prev_stats.checkmates;
        curr_stats.promotions -= prev_stats.promotions;

        std::cout 
            << "depth: " << d 
            << " | nodes: " << nodes[d] 
            << " | captures: " << curr_stats.captures 
            << " | EP: " << curr_stats.ep 
            << " | castles: " << curr_stats.castles 
            << " | checks: " << curr_stats.checks 
            << " | checkmates: " << curr_stats.checkmates 
            << " | promotions: " << curr_stats.promotions 
            << '\n';
    }

#if 0
    std::cout
    << " ------------------------------------------------------------------------------ \n"
    << "make_move / unmake_move errors: \n";

    for(int d = 1; d <= depth; ++d) {
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
#endif

}

bool test_in_check(BoardState board_state, Colour us) 
{
    return board_state.in_check(us);
}
