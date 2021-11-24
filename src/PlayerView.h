#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <SDL2/SDL_image.h>
#include <string>
#include "types.h"
#include "environment.h"

const int NUM_PIECES = 16;
const int MAX_PIECES = 32;
const int NUM_ROWS = 8;

class PlayerView
{
    public: 
        void game_loop();
        void init_pieces(std::string fen);
        void init_piece_clips();
        void init_texture();
        void close_sdl();
        int init_window_and_renderer();
    private:
        SDL_Window* window = NULL;
        SDL_Renderer* renderer = NULL;
        SDL_Texture* texture = NULL;
        SDL_Event e;
        SDL_Rect piece_clips[NUM_PIECES];
        
        BoardPiece board_pieces[MAX_PIECES]; 
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
        bool no_others_dragged(int this_board_piece_idx) ;
        bool in_board_piece(BoardPiece todrag) ;
        void snap(BoardPiece & piece);
        void handle_dragging();
        bool is_piece_ch(char ch);
        Piece fen_to_piece(char ch);
        char piece_to_char(Piece p);
};

#endif
