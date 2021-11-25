#include <stdio.h>
#include <string>
#include <iostream>

#include "types.h"
#include "PlayerView.h"
#include "BoardState.h"

//TODO: 

int main( int argc, char *argv[] )
{
    std::string fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2 ";
    PlayerView game(fen);

    game.init_all();
    game.game_loop();
	return 0;
}
