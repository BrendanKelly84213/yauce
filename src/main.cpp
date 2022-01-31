#include <stdio.h>
#include <string>
#include <iostream>

#include "types.h"
#include "PlayerView.h"

//TODO: 

int main( int argc, char *argv[] )
{
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    game.init_all();
    game.game_loop();

	return 0;
}
