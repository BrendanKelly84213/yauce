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

std::vector<std::string> words(std::string s)
{
    std::stringstream ss(s);
    std::string word;
    std::vector<std::string> vec;

    while(ss >> word) {
        vec.push_back(word);
    }
    return vec;
}

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
    std::string pos5 = "r1b1kb1r/ppp2Npp/8/3pp3/2BnNq2/5P2/PPPP1K1P/R1BQ3R w kq - 0 1";
    std::string inpos;


    BoardState board;
    init_black_tables();

    bool running = true;
    std::string line;
    std::vector<std::string> w;
    Search s;
    while(running) {
        // std::cin >> cmd;
        std::getline(std::cin, line);
        w = words(line);

        if(line == "uci") {
            std::cout << "id name H80000000-1\nid author Brendan\nuciok\n";
        } else if(line == "isready") {
            std::cout << "readyok\n";
        } else if(w[0] == "position") {
            // FIXME: bad data causes segmentation fault 
            if(1 < w.size()) {
                if(w[1] == "fen") {
                    // TODO 
                } else if(w[1] == "startpos")
                    board.init(initial_fen);
            }
        } else if(w[0] == "go") {
            size_t depth = 0; // search depth
            size_t movetime = 0; 
            size_t nodes = 0;
            bool infinite = false;
                                 
            for(size_t i = 1; i < w.size(); ++i) {
                std::string param = w[i];
                std::string val;
                if(param == "depth") {
                    if(i + 1 < w.size()) {
                        depth = stoi(w[i + 1]);
                    }
                } else if(param == "movetime") {
                    if(i + 1 < w.size()) {
                        movetime = stoi(w[i + 1]);
                    }
                } else if(param == "infinite") {
                    infinite = true; 
                }
            }
            
            // TODO: Start searching with given parameters
            // Implement search that can do infinite, search n nodes, etc...

        } else if(w[0] == "stop") {
            s.stop_search();
        }

    }
    return 0;
}
