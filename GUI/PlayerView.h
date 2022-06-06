#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <SDL2/SDL.h>
#include <string>

// For now borrow these structs / enums from BoardState 

enum Piece : int { 
    BQ, BK, BR, BN, BB, BP, 
    WQ, WK, WR, WN, WB, WP,None=-1
};

enum Square : int {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2, 
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4, 
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8, NullSquare = -1
};

constexpr void operator++(Square &s) 
{
    s = (Square)((int)s + 1);
}

enum Colour { White, Black };

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
private: 

    bool running = false;

    size_t tick_count = 0;
    
    SDL_Event e;
    SDL_Window* board_window = NULL;
    SDL_Renderer* board_renderer = NULL;

    int window_w;
    int window_h;
    int square_w;

    Piece squares[64];
    SDL_Texture * piece_graphics[12];

    State state;

    void draw_grid();
    void draw_piece(Piece p, Square s);
    void draw_pieces();
    void on_player_move_piece();
    void draw_available_moves();
    void handle_events(SDL_Event e);
    int xy_to_square(int x, int y);
    void init_squares(std::string fen);
    void init_pieces();

    size_t rank(Square s) const { return s >> 3; };
    size_t file(Square s) const {return s % 8; }

    int square_to_x(Square s) const;
    int square_to_y(Square s) const;

    SDL_Texture * piece_texture(Piece p);

public:
    bool init(std::string fen);
    void run();
};

#endif
