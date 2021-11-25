#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <SDL2/SDL_image.h>
#include <string>
#include "types.h"
#include "environment.h"
#include "BoardState.h"

const int NUM_PIECES = 16;
const int MAX_PIECES = 32;
const int NUM_SQUARES = 64;
const int NUM_ROWS = 8;

// TODO: Add sidebar with info 

class PlayerView
{
    public: 
        PlayerView(std::string _fen) : fen(_fen) {}
        void game_loop();
        void init_all(); // TODO: call init 
        void close_sdl();
    private:
        SDL_Window* window = NULL;
        SDL_Renderer* renderer = NULL;
        SDL_Texture* texture = NULL;
        SDL_Event e;
        SDL_Rect piece_clips[NUM_PIECES];
        
        BoardPiece board_pieces[NUM_SQUARES]; 
        BoardState board_state;
        std::string fen;

        int scr_w = 640;
        int scr_h = 640;
        int rect_w = scr_w / NUM_ROWS;
        bool running = true;
        bool mousedown_on_piece;
        bool mousedown;
        int mx, my;

        void handle_events();
        void draw_grid();
        void draw_pieces();
        bool no_others_dragged(int this_board_piece_idx);
        bool in_board_piece(BoardPiece todrag) ;
        void snap(BoardPiece & piece);
        void handle_dragging();
        void init_piece_clips();
        void init_texture();
        int init_window_and_renderer();
        void init_pieces();
};

#endif
