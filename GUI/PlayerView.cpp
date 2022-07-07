// Handle rendering 
// Handle user IO
//  Dragging
//  Clicking
// Update state 

#include <SDL2/SDL_image.h>
#include <iostream>
#include "PlayerView.h"
#include "../eval.h"

// FIXME: Don't hardcode svg_path
const std::string svg_path[] = {
    "assets/piece/png/cburnett/bQ.png",
    "assets/piece/png/cburnett/bK.png",
    "assets/piece/png/cburnett/bR.png",
    "assets/piece/png/cburnett/bN.png",
    "assets/piece/png/cburnett/bB.png",
    "assets/piece/png/cburnett/bP.png",
    "assets/piece/png/cburnett/wQ.png",
    "assets/piece/png/cburnett/wK.png",
    "assets/piece/png/cburnett/wR.png",
    "assets/piece/png/cburnett/wN.png",
    "assets/piece/png/cburnett/wB.png",
    "assets/piece/png/cburnett/wP.png"
};

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
                SDL_SetRenderDrawColor(board_renderer, 7, 54, 66,0);
            } else {
                SDL_SetRenderDrawColor(board_renderer, 88,110,117,0);
            }
            SDL_RenderFillRect(board_renderer, &square);
        }
    }
}

void PlayerView::draw_piece(BoardPiece bp)
{
    if(bp.p == None) 
        printf("Piece is None, nothing to draw\n");

    int x, y;
    if(!bp.dragging) {
        x = square_to_x(bp.s, square_w, bottom_colour); 
        y = square_to_y(bp.s, square_w, bottom_colour); 
    } else {
        SDL_GetMouseState(&x, &y);
        x = x - (square_w * 0.5);
        y = y - (square_w * 0.5);
    }

    SDL_Texture * texture = piece_textures[bp.p];
    SDL_Rect r = { x, y, square_w, square_w };
    SDL_RenderCopy(board_renderer, texture, NULL, &r);
}

void PlayerView::draw_pieces()
{ 
    for(Square s = a1; s <= h8; ++s) {
        BoardPiece bp = board_pieces[s];
        if(bp.p != None)
            draw_piece(bp);
    }
}

// Draw in available moves
void PlayerView::draw_available_moves()
{
}

SDL_Texture* PlayerView::piece_texture(Piece p)
{
    std::string path = svg_path[p];
    SDL_Surface * surface = IMG_Load(path.c_str());
    if(surface == NULL) {
        printf("Error getting surface from svg: %s, %s\n", path.c_str(), SDL_GetError());
        exit(1);
    }

    SDL_Texture * texture = SDL_CreateTextureFromSurface(board_renderer, surface);
    SDL_FreeSurface(surface);
    if(texture == NULL) {
        printf("Error creating texture from surface: %s\n", SDL_GetError());
        exit(1);
    }
    return texture;
}

void PlayerView::init_piece_textures()
{
    for(Piece p = BQ; p <= WP; ++p) {
        piece_textures[p] = piece_texture(p);
    }
}

bool PlayerView::init(std::string fen, Colour bc, bool bi)
{ 
    update_window();

    // Init SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) 
        return false;

    board_window = SDL_CreateWindow("Chess!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 640, SDL_WINDOW_RESIZABLE);
    if(board_window == NULL)
        return false;

    board_renderer = SDL_CreateRenderer(board_window, -1, SDL_RENDERER_ACCELERATED);
    if(board_renderer == NULL)
        return false;

    // Init image
    IMG_Init(IMG_INIT_PNG);

    // Init engine 
    board.init(fen);
    init_black_tables();
    search.init(4);

    init_pieces();

    init_piece_textures();
     
    bottom_colour = bc;
    board_inverted = bi;
    return true;
} 

void PlayerView::init_pieces()
{
    for(Square s = a1; s <= h8; ++s) {
        Piece p = board.get_piece(s);
        if(p != None) {
            board_pieces[s].init(p, s, square_w, bottom_colour);
        }
    }
}

void PlayerView::update_pieces()
{
    for(Square s = a1; s <= h8; ++s) {
        if(board_pieces[s].dragging) {
            int x, y; 
            SDL_GetMouseState(&x, &y);
            int center_x = x - (0.5 * square_w);
            int center_y = y - (0.5 * square_w);
            board_pieces[s].update_square(center_x, center_y, square_w, bottom_colour);
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

bool PlayerView::drag_selected_piece(int x, int y) 
{
    Square s = xy_to_square(x, y, square_w, bottom_colour);
    BoardPiece& bp = board_pieces[s];
    if(bp.in_piece(x, y, square_w, bottom_colour) && bp.p != None && !bp.dragging) {
        bp.dragging = true; 
        return true;
    }
    return false;
}

bool PlayerView::undrag_selected_piece(int x, int y)
{

    Square piece_id = current_move.from;
    BoardPiece& bp = board_pieces[piece_id];
    if(!bp.dragging)
        return false;

    bp.dragging = false;
    return true;
}

void PlayerView::engine_make_move()
{
}

void PlayerView::player_make_move(BMove m)
{
    Move flag = get_flag(m);
}

bool PlayerView::is_legal(PieceType pt, Square from, Square to) const
{
    Colour us = board.get_side_to_move();
    if(board.get_piece_colour(board.get_piece(from)) != us)
        return false;

    Bitboard to_squares = board.get_to_squares(pt, from, us);
    if(pt == Pawn) {
        // Double pawn push
        if(us == White && rank(from) == 1)
            to_squares |= bit(from + N + N);
        else if(us == Black && rank(from) == 6)
            to_squares |= bit(from + S + S);

        // TODO: En Passant

    }

    if(pt == King) {
        if(board.can_castle(us, OO)) 
            to_squares |= bit(from + E + E);
        if(board.can_castle(us, OOO))
            to_squares |= bit(from + W + W);
    }

    // print(to_squares);
    return bit(to) & to_squares;
} 

void PlayerView::move_piece(Square from, Square to)
{
    BoardPiece copy_bp = board_pieces[from];
    board_pieces[from].p = None;
    board_pieces[to] = copy_bp; 
    board_pieces[to].s = to; 
}

void PlayerView::run()
{
    running = true;
    while(running) {
        if(SDL_PollEvent(&e)) {
            int x, y;
            SDL_GetMouseState(&x,&y);

            if(e.type == SDL_QUIT) {
                running = false;
            } else if(e.type == SDL_MOUSEBUTTONDOWN && !piece_being_dragged) {
                clear_current_move();
                if(drag_selected_piece(x, y)) {
                    piece_being_dragged = true;
                    current_move.from = xy_to_square(x, y, square_w, bottom_colour);
                }
            } else if(e.type == SDL_MOUSEBUTTONUP && piece_being_dragged) {

                // Stop dragging the piece 
                Square piece_id = current_move.from;
                board_pieces[piece_id].dragging = false;
                piece_being_dragged = false;

                Square potential_to = xy_to_square(x, y, square_w, bottom_colour);
                PieceType moving_piecetype = piece_to_piecetype(board_pieces[piece_id].p);
                Move flag = QUIET;
                Colour us = board.get_side_to_move();

                if(editing || is_legal(moving_piecetype, current_move.from, potential_to)) {
                    current_move.to = potential_to;

                    // Flag 
                    if(moving_piecetype == Pawn) {

                        bool from_first_rank = us == White && rank(current_move.from) == 1 || us == Black && rank(current_move.from) == 6;
                        bool double_push = us == White && rank(current_move.to) == 3 || us == Black && rank(current_move.to) == 4;
                        if(from_first_rank && double_push)
                            flag = DOUBLE_PAWN_PUSH;

                        // TODO: En Passant
                        // TODO: Promotions

                    }  

                    if(moving_piecetype == King) {
                        bool white_ks = us == White && current_move.from == e1 && current_move.to == g1;
                        bool black_ks = us == Black && current_move.from == e8 && current_move.to == g8;
                        bool white_qs = us == White && current_move.from == e1 && current_move.to == c1;
                        bool black_qs = us == Black && current_move.from == e8 && current_move.to == c8;
                        if(white_ks || black_ks)
                            flag = OO;
                        else if(white_qs || black_qs)
                            flag = OOO;
                    }

                    // Update board_piece index
                    move_piece(current_move.from, current_move.to);

                    BMove m = move(current_move.from, current_move.to, flag); 

                    // Handle funny moves
                    // Castles, move the rook
                    if(flag == OO) {
                        if(us == White) {
                            move_piece(h1, f1);
                        } else {
                            move_piece(h8, f8);
                        }
                    }

                    if(flag == OOO) {
                        if(us == White) {
                            move_piece(a1, d1);
                        } else {
                            move_piece(a8, d8);
                        }
                    }

                    board.make_move(m);
                } else {
                    board_pieces[piece_id].s = piece_id;
                }
                clear_current_move();
            } else if(e.type == SDL_KEYUP) {
                if(e.key.keysym.sym == SDLK_e) {
                    editing = !editing;
                    if(editing)
                        printf("Editing\n");
                    else 
                        printf("Not Editing\n");
                }
                else if(e.key.keysym.sym == SDLK_ESCAPE)
                    running = false;
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
    }

    SDL_DestroyRenderer(board_renderer);
    SDL_DestroyWindow(board_window);
     
    // Close SDL 
    SDL_Quit(); 
}
