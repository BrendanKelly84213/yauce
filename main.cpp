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

void go(std::istringstream &iss)
{
    s.reset();
    std::string token;
    while(iss >> token) {
        if(token == "depth") iss >> s.depth; 
        else if(token == "movetime") iss >> s.movetime;
        else if(token == "wtime") iss >> s.wtime;
        else if(token == "btime") iss >> s.btime;
        else if(token == "winc") iss >> s.btime;
        else if(token == "binc") iss >> s.btime;
        else if(token == "nodes") iss >> s.nodes;
        else if(token == "infinite") s.infinite = true;
    }

    // search();

    std::thread t(search);
    t.detach();
}

void set_position(std::istringstream &iss)
{
    std::string token;
    iss >> token;

    if(token == "fen") { 
        // TODO
        std::string fen; 
        while(iss >> token && token != "moves") {
           fen += token + " ";            
        }
        board.init(fen);
    } else if(token == "startpos") {
        board.init(startpos); 
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

int main( int argc, char *argv[] )
{
#if 1
    init_black_tables();
    init_table();
    board.init(startpos);

    std::string line;
    std::string token;
    while(std::getline(std::cin, line)) {
        std::istringstream iss(line);
        iss >> token;

        if (token == "uci") {
            std::cout << "id name H80000000-1\nid author Brendan\nuciok\n";
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
        } else {
            std::cout << "I don't know that one.\n";
        }
    }

    return 0;
#else 
    init_black_tables();

    init_table();
    board.init(startpos);

    std::istringstream i("position startpos moves c2c4 c7c5 g1f3 b8c6 b1c3 d7d6 d1c2 c6b4 c2b1 g7g6 e1d1 f8g7 c3d5 c8f5 e2e4 f5d7 d5b4 c5b4 d1e1 d8b6 b1c2 a8d8 f1d3 g8h6 c2d1 e8g8 d1e2 h6g4 d3c2 g7d4 h1f1 d4f6 e1d1 f6g7 a1b1 f8e8 d1e1 a7a6 e2d3 b6c5 d3b3 a6a5 e1e2 a5a4 b3d3 e8f8 e2e1 e7e6 d3e2 c5b6 e2d1 d8a8"); 
    set_position(i);

     // do a search to get some transpositions
    std::istringstream j("depth 4");
    go(j);
            
    std::istringstream k("moves e1e2 g4e5");
    set_position(k);

    BMove moves[256];
    size_t num_moves = psuedo_generator(board, moves);

    auto print = [&]() {
        for(size_t i = 0; i < num_moves; ++i) {
            BMove m = moves[i];
            ZobristKey key = board.generate_key_after_move(m);
            TTItem * entry = s.tt.get(key);
            std::cout 
                << "move " << long_algebraic(m) 
                << " is transposition " << (entry != NULL ? "yes" : "no") << " ";
            if(entry != NULL) {
                std::cout << "(score " << entry->score << " type " << entry->type << " depth " << entry->depth << ") ";
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    };

    print();
    s.sort_moves(moves, num_moves, board, 5);
    print();
#endif
}
