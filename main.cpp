#include <chrono>
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>

#include "search.h"
#include "utils/types.h"
#include "test/perft.h"
#include "utils/conversions.h"
#include "eval.h"

//TODO: 


int main( int argc, char *argv[] )
{
    std::string initial_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string random_fen = "5b2/4p1P1/7p/7q/3k1K2/2p1p1P1/b4Prr/2n1B3 w - - 0 1";
    std::string castle_fen = "rnbqkbnr/pp1pp1pp/2p5/4Pp2/2B5/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1";
    std::string buggy_fen = "2r3k1/1q1nbppp/r3p3/3pP3/pPpP4/P1Q2N2/2RN1PPP/2R4K b - b3 0 23";
    std::string test_special_ep_fen = "8/6bb/8/8/R1pP2k1/4P3/P7/K7 b - d3 0 0";
    std::string kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
    std::string in_check = "rnb1kbnr/pppppppp/8/8/1q6/3P4/PPP1PPPP/RNBQKBNR w - - 0 1";
    std::string pos3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
    std::string pos4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    std::string randompos = "r1bqk1nr/pppp1p1p/2n5/2b3p1/2BpP3/2N2N2/PPP2PPP/R1BQK2R w KQkq - 0 6";
    std::string pos5 = "r1b1k1nr/1p3ppp/p2bp3/3pq3/8/2NBPQ2/PPPB1PPP/R4RK1 w kq - 0 1";

    BoardState board;
    board.init(buggy_fen);
    init_black_tables();
    board.print_squares();

#if 1

    bool running = true;
    while(running) {
        std::string cmd;
        std::string param1;
        std::cin >> cmd >> param1;

        if(strcmp(cmd.c_str(), "stop") == 0) {
            running = false;
        }
    
        if(strcmp(cmd.c_str(), "go") == 0) {
            Line pline;
            Search s(1000);
            char paramc = param1.c_str()[0];
            size_t param = (size_t)(paramc - 48);
            printf("%lu\n", param);

            auto search_start = std::chrono::steady_clock::now();
            int score = s.search(board, param, &pline);
            auto search_end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = search_end - search_start;

            printf("Searched %lu nodes at depth %lu in %fs. Score: %d \n", s.get_nodes_searched(), param, elapsed.count(), score);       
            s.print_line(board, pline);
        }

        if(strcmp(cmd.c_str(), "move") == 0) {
           Square from = string_to_square(param1.substr(0,2)); 
           Square to = string_to_square(param1.substr(2,2)); 
           printf("%s %s\n", square_to_str(from).c_str(), square_to_str(to).c_str());
           printf("%d %d\n", from, to);
           BMove m = move(from, to, QUIET);
           board.make_move(m);
           board.print_squares();
        }

        // Search s(2);
        
        // std::cout << "Evaluating..." << '\n';
        // board.print_squares();

        // auto start = std::chrono::steady_clock::now();
        // s.iterative_search(board);
        // auto end = std::chrono::steady_clock::now();

        // Duration elapsed = end - start; 

        // std::cout << "searched  depth " << s.get_depth_searched() - 1 << " and " << s.get_nodes_searched() << " nodes in " << elapsed.count() << "s" << '\n'; 

    }
#endif
    return 0;
}
