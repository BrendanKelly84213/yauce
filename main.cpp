#include <chrono>
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <thread>

#include "search.h"
#include "utils/types.h"
#include "test/perft.h"
#include "utils/conversions.h"
#include "eval.h"
#include "transposition.h" 

std::string startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
std::string random_fen = "5b2/4p1P1/7p/7q/3k1K2/2p1p1P1/b4Prr/2n1B3 w - - 0 1";
std::string castle_fen = "rnbqkbnr/pp1pp1pp/2p5/4Pp2/2B5/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1";
std::string buggy_fen = "2r3k1/1q1nbppp/r3p3/3pP3/pPpP4/P1Q2N2/2RN1PPP/2R4K b - b3 0 23";
std::string test_special_ep_fen = "8/6bb/8/8/R1pP2k1/4P3/P7/K7 b - d3 0 0";
std::string kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
std::string in_check = "rnb1kbnr/pppppppp/8/8/1q6/3P4/PPP1PPPP/RNBQKBNR w - - 0 1";
std::string pos3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
std::string pos4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
std::string randompos = "r1bqk1nr/pppp1p1p/2n5/2b3p1/2BpP3/2N2N2/PPP2PPP/R1BQK2R w KQkq - 0 6";
std::string pos5 = "r1b1kb1r/ppp2Npp/8/3pp3/2BnNq2/5P2/PPPP1K1P/R1BQ3R w kq - 0 1";
std::string inpos;

BoardState board;
Search s;

void search()
{
    s.iterative_search(board); 
}

void go(std::istringstream & iss)
{
    s.reset();
    std::string token;
    while(iss >> token) {
        if(token == "depth") iss >> s.depth; 
        else if(token == "movetime") iss >> s.movetime;
        else if(token == "wtime") iss >> s.wtime;
        else if(token == "btime") iss >> s.btime;
        else if(token == "winc") iss >> s.winc;
        else if(token == "binc") iss >> s.binc;
        else if(token == "nodes") iss >> s.nodes;
        else if(token == "infinite") s.infinite = true;
    }

    std::thread t(search);
    t.detach();
}

void set_position(std::istringstream & iss)
{
    std::string token;
    iss >> token;

    if(token == "fen") { 
        std::string fen; 
        while(iss >> token && token != "moves") {
           fen += token + " ";            
        }
        board.init(fen);
    } else if(token == "startpos") {
        board.init(startpos); 
    } else if(token == "kiwipete") {
        board.init(kiwipete);
    }

    while(iss >> token) {
        if(token == "moves") 
            continue;     
        
        BMove moves[256];
        size_t num_moves = psuedo_generator(board, moves);
        for(size_t i = 0; i < num_moves; ++i) {
            BMove m = moves[i];
            if(long_algebraic(m) == token) {
                board.make_move(m);
                break;
            }   
        }
    }
}

void do_perft(std::istringstream & iss)
{

    std::string token;
    int depth = 0;
    while(iss >> token) {
        if(token == "depth") 
            iss >> depth;
    }

    auto p = [&]() {
        print_perft(depth, board);  
    };
    
    std::thread t(p);
    t.detach();
}

void help()
{
    std::cout << "YAU (Chess Engine) by Brendan\n";
    std::cout << "Supported (UCI) commands:\n";
    std::cout << "  uci          - Initialize UCI protocol\n";
    std::cout << "  ucinewgame   - Start a new game\n";
    std::cout << "  isready      - Check if the engine is ready\n";
    std::cout << "  stop         - Stop the current search\n";
    std::cout << "  go           - Start a search for the best move\n";
    std::cout << "  quit         - Exit the program\n";
    std::cout << "  position     - Set up a board position with a fen\n";
    std::cout << "  setoption    - Set an engine option (not implemented yet)\n";
    std::cout << "  perft        - Perform a perft test\n";
    
    std::cout << "Non UCI commands: \n";
    std::cout << "  printboard   - Print the current board\n";
    std::cout << "  help         - Print this message\n";
}

int main( int argc, char *argv[] )
{
    init_black_tables();
    init_table();
    s.init_mvvlva();
    board.init(startpos);

    help();

    std::string line;
    std::string token;
    while(std::getline(std::cin, line)) {
        std::istringstream iss(line);
        iss >> token;

        if (token == "uci") {
            std::cout << "id name yauce\nid author Brendan\nuciok\n";
        } else if (token == "ucinewgame") {
            board.init(startpos);
        } else if (token == "isready") {
            std::cout << "readyok" << '\n';
        } else if (token == "stop") {
            s.stop_search();
        } else if (token == "go") {
            go(iss);
        } else if (token == "quit") {
            return 0;
        } else if (token == "position") {
            set_position(iss);
        } else if (token == "setoption") {
            std::cout << "to be implemented\n"; 
        } else if (token == "printboard"){
            board.print_squares();
        } else if (token == "perft") {
            do_perft(iss);
        } else if (token == "help") {
            help();    
        } else {
            std::cout << "I don't know that one.\n";
        }
    }

    return 0;
}
