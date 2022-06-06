// Handle rendering 
// Handle user IO
//  Dragging
//  Clicking
// Update state 

// Seperate windows for board and info
// Show available moves on piece selection

#include "PlayerView.h"
#include <SDL2/SDL_image.h>

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

// Top left hand corner of square in 
int PlayerView::square_to_x(Square s) const 
{
    size_t f = file(s);
    return f * square_w;
}

int PlayerView::square_to_y(Square s) const 
{
    size_t r = rank(s);
    return (7 - r) * square_w;
}

// BQ, BK, BR, BN, BB, BP, 
// WQ, WK, WR, WN, WB, WP,None=-1

std::string svg_path[] = {
    "assets/piece/svg/pixel/bQ.svg",
    "assets/piece/svg/pixel/bK.svg",
    "assets/piece/svg/pixel/bR.svg",
    "assets/piece/svg/pixel/bN.svg",
    "assets/piece/svg/pixel/bB.svg",
    "assets/piece/svg/pixel/bP.svg",
    "assets/piece/svg/pixel/wQ.svg",
    "assets/piece/svg/pixel/wK.svg",
    "assets/piece/svg/pixel/wR.svg",
    "assets/piece/svg/pixel/wN.svg",
    "assets/piece/svg/pixel/wB.svg",
    "assets/piece/svg/pixel/wP.svg"
};

SDL_Texture * PlayerView::piece_texture(Piece p)
{
    std::string path = svg_path[p];
    printf("Got path: %s\n", path.c_str());
    SDL_Surface * surface = IMG_Load(path.c_str());
    if(surface == NULL) {
        printf("Error getting surface from svg: %s, %s\n", path.c_str(), SDL_GetError());
        exit(1);
        return NULL;
    }
    SDL_Texture * texture = SDL_CreateTextureFromSurface(board_renderer, surface);
    SDL_FreeSurface(surface);
    if(texture == NULL) {
        printf("Error creating texture from surface: %s\n", SDL_GetError());
        exit(1);
        return NULL;
    }
    return texture;
}

void PlayerView::draw_piece(Piece p, Square s)
{
    int x = square_to_x(s);
    int y = square_to_y(s);

    SDL_Texture * texture = piece_graphics[p];

    SDL_Rect rect = { x, y, square_w, square_w };
    SDL_RenderCopy(board_renderer, texture, NULL, &rect);
}

void PlayerView::draw_pieces()
{ 
    for(Square s = a1; s <= h8; ++s) {
        Piece p = squares[s];
        if(p != None)
            draw_piece(p, s);
    }
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

    // Init image
    IMG_Init(IMG_INIT_PNG);

    // Init position
    init_squares(fen);
    init_pieces();
     
    return true;
} 

void PlayerView::init_pieces()
{
    for(Square s = a1; s <= h8; ++s) {
        Piece p = squares[s];
        if(p != None) {
            piece_graphics[p] =  piece_texture(p);
        }
    }
}

void PlayerView::run()
{
    running = true;
    while(running) {

        uint64_t start = SDL_GetPerformanceCounter();

        // Handle events 
        if(SDL_PollEvent(&e)) {
            handle_events(e);
        }
        // Rendering
        SDL_SetRenderDrawColor(board_renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(board_renderer);
        draw_grid();
        draw_pieces(); 
        /* draw_available_moves(); */
        SDL_RenderPresent(board_renderer);

        uint64_t end = SDL_GetPerformanceCounter();

        float elapsed = (end - start) / (float)(SDL_GetPerformanceFrequency() * 1000.0f);
        
        // Cap frames
        SDL_Delay(floor(16.66f - elapsed));
    }

    SDL_DestroyRenderer(board_renderer);
    SDL_DestroyWindow(board_window);
     
    // Close SDL 
    SDL_Quit(); 
}

// Following section borrowed from BoardState.h
// In the future perhaps move to shared utility file, includes, something

enum Section 
{ 
    Board, 
    SideToMove, 
    CastlingRights, 
    EPSquare, 
    HalfmoveClock, 
    FullmoveCounter 
};

static bool is_piece_ch(char ch) 
{
    return (
            ch == 'p'
         || ch == 'r'
         || ch == 'q'
         || ch == 'k'
         || ch == 'n'
         || ch == 'b'
         || ch == 'P'
         || ch == 'R'
         || ch == 'Q'
         || ch == 'K'
         || ch == 'N'
         || ch == 'B'
    );
}

static Piece fen_to_piece(char ch) 
{
    switch(ch) {
        case 'p': 
            return BP;
        case 'r':
            return BR;
        case 'q':
            return BQ;
        case 'k':
            return BK;
        case 'n':
            return BN;
        case 'b':
            return BB;
        case 'P':
            return WP;
        case 'R':    
            return WR;
        case 'Q':    
            return WQ;
        case 'K':    
            return WK;
        case 'N':    
            return WN;
        case 'B':    
            return WB;
        default: break;
    }
    return None;
}

constexpr Square square(int rank, int file)
{
    return (Square)(rank * 8 + file);
}

void PlayerView::init_squares(std::string fen)
{
    // Parse out pieces 
    int rank=7;
    int file=0;
    int i=0;
    int section=0;
    
    for(int i=a1; i<=h8; ++i) {
        squares[i] = None;
    }

    while(fen[i] != ' ') {
        int sq = (rank)*8 + file;

        if(fen[i] == '/') {
            rank--;
            file=0;
        }

        if(fen[i] >= '0' && fen[i] <= '8') {
            file += (fen[i] - 0x30);
        }

        if(is_piece_ch(fen[i])) {
            squares[sq] = fen_to_piece(fen[i]);
            file++;
        } 

        i++;
    }

    section++;

    // Parse out info
    std::string info = fen.substr(i + 1, fen.length());
    for(int i=0; i<info.length(); ++i) {
        if(info[i] == ' ') {
            section++;
            /* continue; */
        }
        switch(section) {
            case SideToMove: 
                if(info[i] == 'w') {
                    state.side_to_move = White;
                } else {
                    state.side_to_move = Black;
                }
                break;
            case CastlingRights:
                switch(info[i]) {
                    case '-':
                        state.w_castle_ks = false; 
                        state.w_castle_qs = false; 
                        state.b_castle_ks = false; 
                        state.b_castle_qs = false; 
                        break;
                    case 'k':
                        state.b_castle_ks = true;
                        break;
                    case 'q':
                        state.b_castle_qs = true;
                        break;
                    case 'K': 
                        state.w_castle_ks = true;
                        break;
                    case 'Q':
                        state.w_castle_qs = true;
                        break;
                    default: break;
                }
                break;
            case EPSquare:
                if(info[i] == '-') {
                    state.ep_square = NullSquare;
                } else if(info[i] >= 'a' && info[i] <= 'h') {
                    int ep_file = static_cast<int>(info[i] - 0x61);
                    int ep_rank = state.side_to_move == White ? 4 : 5;
                    state.ep_square = square(ep_rank, ep_file);
                } 
                break;
            case HalfmoveClock:
                state.halfmove_clock = static_cast<int>(info[i] - 0x30);
                break;
            case FullmoveCounter:
                state.ply_count = static_cast<int>(info[i] - 0x30);
                break;
            default: break;
        }
    }
}
