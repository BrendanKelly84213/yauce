#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <SDL2/SDL.h>
#include <string>
#include "BoardPiece.h"

struct State {
    Colour side_to_move = White; 
    // TODO: Castle can be a lot more lightweight...
    bool w_castle_ks = false; 
    bool w_castle_qs = false; 
    bool b_castle_ks = false; 
    bool b_castle_qs = false; 
    Square ep_square = NullSquare;
    // FIXME: int -> size_t
    int halfmove_clock = 0;
    int ply_count = 0;     
    Piece last_captured = None;

    bool operator==(State &b) const;
};

class PlayerView {
public:

    bool init(std::string fen);
    void run();

private: 

    bool running = false;
    bool piece_being_dragged = false;

    SDL_Event e;
    SDL_Window * board_window = NULL;
    SDL_Renderer * board_renderer = NULL;

    int window_w;
    int window_h;
    int square_w;

    Piece squares[64];
    BoardPiece board_pieces[32]; 
    SDL_Texture * piece_graphics[12];

    State state;

    void draw_grid();
    void draw_piece(BoardPiece p);
    void draw_pieces();
    void on_player_move_piece();
    void draw_available_moves();
    void handle_events(SDL_Event e);
    void init_squares(std::string fen);
    void init_pieces();
    void update_pieces();
    void update_window(); 

    void set_dragging(bool dragging);


    Square xy_to_square(int x, int y);

    SDL_Texture * piece_texture(Piece p);
};

#endif
