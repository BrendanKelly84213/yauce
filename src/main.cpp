#include <stdio.h>
#include <string>
#include <iostream>

#include "types.h"
#include "PlayerView.h"

//TODO: 

int main( int argc, char *argv[] )
{
    PlayerView game;
    std::string fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";

    game.init_pieces(fen);
    game.init_piece_clips(); 
    game.init_window_and_renderer();
    game.init_texture();

    game.game_loop();

	return 0;
}
