// Handle rendering 
// Handle user IO
//  Dragging
//  Clicking
// Update state 

#include <SDL2/SDL_image.h>
#include <iostream>
#include "PlayerView.h"
#include "../eval.h"

void PlayerView::draw_grid()
{
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


// BQ, BK, BR, BN, BB, BP, 
// WQ, WK, WR, WN, WB, WP,None=-1

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

void PlayerView::draw_piece(BoardPiece bp)
{
    SDL_Texture * texture = bp.get_texture();
    SDL_Rect r = bp.get_rect();

    SDL_RenderCopy(board_renderer, texture, NULL, &r);
}

void PlayerView::draw_pieces()
{ 
    for(size_t i = 0; i < 32; ++i) {
        if(board_pieces[i].get_piece() != None)
            draw_piece(board_pieces[i]);
    }
}

void PlayerView::on_player_move_piece()
{
    // Something like this...
}

// Draw in available moves
void PlayerView::draw_available_moves()
{
}

void PlayerView::handle_events(SDL_Event e)
{
}

bool PlayerView::init(std::string fen)
{ 
    update_window();

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

    // Init engine 
    board.init(fen);
    init_black_tables();
    search.init(1);
     
    return true;
} 

void PlayerView::init_pieces()
{
    size_t i = 0;
    for(Square s = a1; s <= h8; ++s) {
        Piece p = squares[s];
        if(p != None) {
            board_pieces[i].init(p, s, square_w, board_renderer);
            i++;
        }
    }
}

void PlayerView::update_pieces()
{
    for(size_t i = 0; i < 32; ++i) {
        if(board_pieces[i].is_being_dragged()) {
            int x, y; 
            SDL_GetMouseState(&x, &y);
            int center_x = x - (0.5 * square_w);
            int center_y = y - (0.5 * square_w);
            board_pieces[i].update(center_x, center_y, square_w);
        } else {
            board_pieces[i].update(square_w);
        }
    }
}

void PlayerView::update_window()
{
    SDL_GetWindowSize(board_window, &window_w, &window_h);
    square_w = window_w < window_h ? window_w >> 3 : window_h >> 3;
}

void PlayerView::clear_current_move()
{
    current_move.from = NullSquare;
    current_move.to = NullSquare;
}

void PlayerView::set_dragging(bool dragging) 
{
    int x, y;
    SDL_GetMouseState(&x, &y);
    for(size_t i = 0; i < 32; ++i) {
        BoardPiece bp = board_pieces[i];
        if(bp.get_piece() != None && bp.is_being_dragged() != dragging ) {
            if(bp.in_piece(x, y)) {
                Square sq = xy_to_square(x, y, square_w);
                board_pieces[i].set_dragging(dragging);
                piece_being_dragged = dragging;

                // Set the current move 
                if(dragging) 
                    current_move.from = sq;
                else 
                    current_move.to = sq;
            }
        }
    }
}

void PlayerView::engine_make_move()
{
    BMove reply = search.iterative_search(board);
    Square from = get_from(reply);
    Square to = get_to(reply);
    Colour us = board.get_side_to_move();
    for(size_t i = 0; i < 32; ++i) {
        BoardPiece bp = board_pieces[i];
        Piece p = bp.get_piece();

        if(board_pieces[i].get_square() == from) {
            board_pieces[i].update(to, square_w);
        }

        if(p != None && bp.get_square() == to && board.get_piece_colour(p) != us)
            board_pieces[i].remove();
    }
    board.make_move(reply);
}

void PlayerView::player_make_move()
{
    printf("moved %s%s\n", square_to_str(current_move.from).c_str(), square_to_str(current_move.to).c_str());

    BMove m = move(current_move.from, current_move.to, QUIET);
    Colour us = board.get_side_to_move();
    board.make_move(m);
    for(size_t i = 0; i < 32; ++i) {
        BoardPiece bp = board_pieces[i];
        Piece p = bp.get_piece();
        if(p != None && bp.get_square() == current_move.to && board.get_piece_colour(p) != us)
            board_pieces[i].remove();
    }

}

void PlayerView::run()
{
    running = true;
    while(running) {

        uint64_t start = SDL_GetPerformanceCounter();

        // Handle events 
        if(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                running = false;
            } else if(e.type == SDL_MOUSEBUTTONDOWN && !piece_being_dragged) {
                clear_current_move();
                set_dragging(true);
            } else if(e.type == SDL_MOUSEBUTTONUP && piece_being_dragged) {
                set_dragging(false);
                player_make_move();
            }
        }

        // Updating

        update_window();
        update_pieces();

        // Rendering
        SDL_SetRenderDrawColor(board_renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(board_renderer);
        draw_grid();
        draw_pieces(); 
        SDL_RenderPresent(board_renderer);

        if(board.get_side_to_move() != state.side_to_move)
            engine_make_move();

        uint64_t end = SDL_GetPerformanceCounter();

        float elapsed = (end - start) / (float)(SDL_GetPerformanceFrequency() * 1000.0f);

        // Cap frames
        // SDL_Delay(floor(16.66f - elapsed));
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
