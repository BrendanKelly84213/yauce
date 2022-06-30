#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <SDL2/SDL.h>
#include <string>
#include "../BoardState.h"
#include "BoardPiece.h"
#include "../search.h"

struct GMove {
    Square from, to; 
};

class PlayerView {
public:

    bool init(std::string fen, Colour pc, bool bi);
    void run();

private: 

    bool running = false;
    bool piece_being_dragged = false;
    bool editing = true; // are we currently creating a board 

    Colour player_colour;
    Colour bottom_colour;
    bool board_inverted = false;

    SDL_Event e;
    SDL_Window * board_window = NULL;
    SDL_Renderer * board_renderer = NULL;

    int window_w;
    int window_h;
    int square_w;

    BoardPiece board_pieces[64]; 
    SDL_Texture * piece_textures[12];

    State state;

    // Engine 
    BoardState board;
    Search search;

    GMove current_move;

    void draw_grid();
    void draw_piece(BoardPiece bp);
    void draw_pieces();
    void on_player_move_piece();
    void draw_available_moves();
    void handle_events(SDL_Event e);
    void init_squares(std::string fen);
    void init_pieces();
    void init_piece_textures();
    void update_pieces();
    void update_window(); 

    void engine_make_move();
    void player_make_move();

    void clear_current_move();

    bool drag_selected_piece(int x, int y);
    bool undrag_selected_piece(int x, int y);

    SDL_Texture * piece_texture(Piece p);
};

#endif
