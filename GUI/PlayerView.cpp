// Handle rendering 
// Handle user IO
//  Dragging
//  Clicking
// Update state 

// Seperate windows for board and info
// Show available moves on piece selection

#include "PlayerView.h"

enum PieceIndex {
    WKIdx,
    WQIdx,
    WBIdx,
    WNIdx,
    WRIdx,
    WPIdx,
    BKIdx,
    BQIdx,
    BBIdx,
    BNIdx,
    BRIdx,
    BPIdx,
};

// Init piece clipes relative to piece_sprite_sheet texture
void init_piece_clips()
{
}

void PlayerView::draw_grid()
{
    SDL_GetWindowSize(board_window, &window_w, &window_h);
    square_w = window_w < window_h ? window_w >> 3 : window_h >> 3;
    for(int i=0; i<8; ++i) {
        for(int j=0; j<8; ++j){
            SDL_Rect square;
            square.w = square_w;
            square.h = square_w;
            square.x = i*square_w;
            square.y = j*square_w;

            if(((i+j)%2)){ 
                SDL_SetRenderDrawColor(board_renderer, 15,50,20,0);
            } else {
                SDL_SetRenderDrawColor(board_renderer, 200,200,255,0);
            }
            SDL_RenderFillRect(board_renderer, &square);
        }
    }
}

void PlayerView::draw_pieces()
{ 
    /* SDL_RenderCopy(board_renderer, piece_sprite_sheet, &piece_clip, &dest_rect); */
}

void PlayerView::on_player_move_piece()
{
    // Something like this...
}

// XY coordinates on the screen to square on the board 
int PlayerView::xy_to_square(int x, int y)
{
    int rank = 7 - (int)(y / square_w);
    int file = (int)(x / square_w);
    return rank * 8 + file;
}

// Draw in available moves
void PlayerView::draw_available_moves()
{
}

void PlayerView::handle_events(SDL_Event e)
{
    if(e.type == SDL_QUIT) 
        running = false;
}

bool PlayerView::init(std::string fen)
{ 
    SDL_GetWindowSize(board_window, &window_w, &window_h);

    // Init SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }
    if(SDL_CreateWindowAndRenderer(640, 640, SDL_WINDOW_RESIZABLE, &board_window, &board_renderer)) {
        return false;
    }
     
    return true;
} 

void PlayerView::run()
{
    running = true;
    while(running) {
        // Handle events 
        if(SDL_PollEvent(&e)) {
            handle_events(e);
        }
        // Rendering
        SDL_SetRenderDrawColor(board_renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(board_renderer);
        draw_grid();
        /* draw_pieces(); */
        /* draw_available_moves(); */
        SDL_RenderPresent(board_renderer);
    }

    SDL_DestroyRenderer(board_renderer);
    SDL_DestroyWindow(board_window);
     
    // Close SDL 
    SDL_Quit(); 
}

