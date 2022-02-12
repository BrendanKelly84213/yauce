#include <stdio.h>
#include <string>
#include <iostream>

#include "utils/types.h"
#include "GUI/PlayerView.h"

//TODO: 


int main( int argc, char *argv[] )
{
    std::string initial_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string random_fen = "5b2/4p1P1/7p/7q/3k1K2/2p1p1P1/b4Prr/2n1B3 w - - 0 1";
    std::string castle_fen = "rnbqkbnr/pp1pp1pp/2p5/4Pp2/2B5/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1";
    std::string buggy_fen = "2r3k1/1q1nbppp/r3p3/3pP3/pPpP4/P1Q2N2/2RN1PPP/2R4K b - b3 0 23";
    std::string test_special_ep_fen = "8/6bb/8/8/R1pP2k1/4P3/P7/K7 b - d3 0 0";

    init_generator();
    
    if(!playerview::init(test_special_ep_fen)) {
        return 1;
    }

    playerview::run();


	return 0;
}
