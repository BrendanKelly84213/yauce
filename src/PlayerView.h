#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "environment.h"
#include "Generator.h"
#include "Texture.h"

// TODO: Add sidebar with info 
// Texture class 
//
const int NUM_PIECES = 16;
const int MAX_PIECES = 32;
const int NUM_SQUARES = 64;
const int NUM_ROWS = 8;

class PlayerView
{
public: 
    PlayerView(std::string _fen) : fen(_fen) {}
    void game_loop();
    void init_all(); // TODO: call init 
    void close_sdl();
private:
    SDL_Event e;
    SDL_Rect piece_clips[NUM_PIECES];
    SDL_Rect board_viewport;
    SDL_Rect info_viewport;

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    
    BoardPiece board_pieces[NUM_SQUARES]; 
    BoardState board_state;
    std::string fen;

    std::vector<Move> move_vec;
    Move current_move;

    TTF_Font* font;

    Texture pieces_texture;

    Texture side_to_move_texture;
    Texture w_castling_ks_texture;
    Texture w_castling_qs_texture;
    Texture b_castling_ks_texture;
    Texture b_castling_qs_texture;
    Texture ep_file_texture;
    Texture halfmove_clock_texture;
    Texture ply_count_texture;

    int scr_w = SCREEN_WIDTH;
    int scr_h = SCREEN_HEIGHT;
    int rect_w;
    bool running = true;
    bool mousedown_on_piece;
    bool mousedown;
    int mx, my;

    void handle_events();
    void draw_grid();
    void draw_pieces();
    void draw_info();

    void handle_dragging();
    bool no_others_dragged(int this_board_piece_idx);
    bool in_board_piece(BoardPiece todrag) ;
    void snap(BoardPiece & piece);

    void init_piece_clips();
    void init_texture();
    void init_viewports();
    int init_window_and_renderer();
    void init_pieces();
    bool update_info();
    void update_moves();
    bool valid_move(Square from, Square to);
    void on_player_make_move(Piece piece);

    void free_textures();
};

#endif
