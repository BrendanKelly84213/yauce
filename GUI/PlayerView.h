#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>

class PlayerView {
private: 

    bool running = false;
    
    SDL_Event e;
    SDL_Window* board_window = NULL;
    SDL_Renderer* board_renderer = NULL;

    int window_w;
    int window_h;
    int square_w;

    void draw_grid();
    void draw_pieces();
    void on_player_move_piece();
    void draw_available_moves();
    void handle_events(SDL_Event e);
    int xy_to_square(int x, int y);

public:
    bool init(std::string fen);
    void run();
};


#endif
