// Handle rendering 
// Handle user IO
//  Dragging
//  Clicking
// Update state 

// Seperate windows for board and info
// Show available moves on piece selection

#include "PlayerView.h"

namespace playerview { 

SDL_Event e;
SDL_Window* board_window = NULL;
SDL_Renderer* board_renderer = NULL;
bool running = false;
BoardState board_state;
SDL_Texture* piece_sprite_sheet;
SDL_Rect piece_clips[12]; // clippings from piece_sprite_sheet
int window_w;
int window_h;
int square_w;
BMove available_moves[128];
BMove selected_piece_moves[28] = {};
BMove square_moves[64][28];

enum PieceClipIndex {
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
    for(int p=0; p<12; ++p) {
        piece_clips[p].w = 60;
        piece_clips[p].h = 60;

        if(p >=0 && p < 6) {
            piece_clips[p].x = p*60;
            piece_clips[p].y = 0;
        } else {
            piece_clips[p].x = (p-6)*60;
            piece_clips[p].y = 60;
        }
    }
}

void draw_grid()
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

void draw_pieces()
{ 
    for(int s=0; s<64; ++s) {
        int p = (int)board_state.squares[s];
        int piece_clip_index = p; 
        int file = s % 8;
        int rank = s >> 3;
        int x = file*square_w;
        int y = 7*square_w - rank*square_w;
        SDL_Rect piece_clip = piece_clips[piece_clip_index];
        SDL_Rect dest_rect = { x, y, square_w, square_w };
        if(p >= 0) {
            SDL_RenderCopy(board_renderer, piece_sprite_sheet, &piece_clip, &dest_rect);
        } 
    }
}

void on_player_move_piece()
{
    // Something like this...
    BMove m = 0x00; 
    board_state.make_move(m);
}

int xy_to_square(int x, int y)
{
    int rank = 7 - (int)(y / square_w);
    int file = (int)(x / square_w);
    return rank * 8 + file;
}

Colour piece_colour(int piece)
{
   if(piece >= 0 && piece <= 5) {
       return Black;
   } 
   return White;
}

void set_available_moves(int sq, int piece) 
{
    int pt=-1;
    int i=0;
    Bitboard to_squares=0;
    if(piece >= 0 && board_state.side_to_move == piece_colour(piece)) {
        pt = conversions::piece_to_piecetype(piece);
        to_squares = get_to_squares(pt, sq, board_state); 
    }
    for(int i=0; i<28; ++i) {
        selected_piece_moves[i] = -1;
    }
    if(pt == King && can_castle_ks(board_state)) {
        std::cout << "can castle ks" << '\n';
        selected_piece_moves[i] = board_state.side_to_move == White ? g1 : g8;
        ++i;
    }
    if(pt == King && can_castle_qs(board_state)) {
        std::cout << "can castle qs" << '\n';
        selected_piece_moves[i] = board_state.side_to_move == White ? d1 : d8;
        ++i;
        selected_piece_moves[i] = board_state.side_to_move == White ? c1 : c8;
        ++i;
    }
    print(to_squares);
    while(to_squares) {
        int s = pop_bit(to_squares);
        selected_piece_moves[i] = s;
        i++;
    }

    std::string p = conversions::piece_to_str(piece);
    std::string s = conversions::square_to_str(sq);
    std::cout << "square: " << s << " piece: " 
        << p << " piecetype: " << conversions::piecetype_to_str(pt) << '\n';
}

void draw_available_moves()
{
    for(int i=0; i<28; ++i) {
        int s = selected_piece_moves[i];
        int rank = s >> 3;
        int file = s % 8;
        int x = file * square_w;
        int y = square_w*(7 - rank);

        SDL_Rect square;
        square.x = x;
        square.y = y;
        square.w = square_w;
        square.h = square_w;

        SDL_SetRenderDrawColor(board_renderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderFillRect(board_renderer, &square);
    }
}

void handle_events(SDL_Event e)
{
    int x,y;
    if(e.type == SDL_QUIT) {
        running = false;
    } else if(e.type == SDL_MOUSEBUTTONDOWN) {
        SDL_GetMouseState(&x,&y);
        int s = xy_to_square(x,y);
        int p = (int)board_state.squares[s];
        set_available_moves(s, p);
    }
}

bool init(std::string fen)
{ 
    SDL_GetWindowSize(board_window, &window_w, &window_h);

    // Init SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }
    if(SDL_CreateWindowAndRenderer(640, 640, SDL_WINDOW_RESIZABLE, &board_window, &board_renderer)) {
        return false;
    }

    board_state.init(fen); 
    // Load surface 
    SDL_Surface* surface = IMG_Load("../assets/ChessPiecesArray.png");
    piece_sprite_sheet = SDL_CreateTextureFromSurface(board_renderer, surface);
    SDL_FreeSurface(surface);
    surface = NULL;
    
    // Cut out piece clips from sprite sheet texture 
    init_piece_clips();
     
    return true;
} 

void run()
{

    for(int i=0; i<28; ++i) {
        selected_piece_moves[i] = -1;
    }
    // For now...? In future: on_player_make_move
    BMove * moves_ptr;
    moves_ptr = generator(board_state);

    int i=0;
    while(*moves_ptr != 0x00) {
        BMove m = *moves_ptr++;
        int from = (m << 6) & 0x3f;
        available_moves[i] = m;
        i++;
    }
    moves_ptr = NULL;

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
        draw_pieces();
        draw_available_moves();
        SDL_RenderPresent(board_renderer);
        
        // Testing
    }
    SDL_DestroyRenderer(board_renderer);
    SDL_DestroyWindow(board_window);
    // Free textures
    SDL_DestroyTexture(piece_sprite_sheet); 
    piece_sprite_sheet = NULL; 
     
    // Close SDL 
    SDL_Quit(); 
}

}
