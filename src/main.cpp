#include <stdio.h>
#include <string>
#include <iostream>

#include "types.h"
#include "PlayerView.h"

//TODO: 

int main( int argc, char *argv[] )
{
    std::string fen = "1K6/8/4B2r/P1p3pr/6P1/Qb1Pb3/2k1pB2/3R4 w - - 0 1";
    PlayerView game(fen);

    game.init_all();
    game.game_loop();

	return 0;
}
