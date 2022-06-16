// Handle rendering 
// Handle user IO
//  Dragging
//  Clicking
// Update state 

#include <SDL2/SDL_image.h>
#include <iostream>
#include "PlayerView.h"
#include "../eval.h"

constexpr int SCREEN_FPS = 60;
constexpr int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

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

// Draw in available moves
void PlayerView::draw_available_moves()
{
}

bool PlayerView::init(std::string fen, Colour pc, bool bi)
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
    search.init(3);

    init_pieces();

     
    player_colour = pc;
    board_inverted = bi;
    return true;
} 

void PlayerView::init_pieces()
{
    size_t i = 0;
    for(Square s = a1; s <= h8; ++s) {
        Piece p = board.get_piece(s);
        if(p != None) {
            board_pieces[i].init(p, s, square_w, board_renderer, player_colour);
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
            board_pieces[i].update(center_x, center_y, square_w, player_colour);
        } else {
            board_pieces[i].update(square_w, player_colour);
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
                Square sq = xy_to_square(x, y, square_w, player_colour);
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
    Move flag = get_flag(reply);
    Colour us = board.get_side_to_move();
    for(size_t i = 0; i < 32; ++i) {
        BoardPiece bp = board_pieces[i];
        Piece p = bp.get_piece();

        bool capture = p != None && bp.get_square() == to && board.get_piece_colour(p) != us;
        bool black_castle_kingside_rook = flag == OO && us == Black && bp.get_square() == h8;
        bool white_castle_kingside_rook = flag == OO && us == White && bp.get_square() == h1;
        bool black_castle_queenside_rook = flag == OOO && us == Black && bp.get_square() == a8;
        bool white_castle_queenside_rook = flag == OOO && us == White && bp.get_square() == a1;
        bool promotion = flag >= PROMOTE_QUEEN && flag <= PROMOTE_BISHOP && piece_to_piecetype(p) == Pawn && on_opposite_rank(to, us);
        bool en_passant = 
            flag == EN_PASSANT 
            && piece_to_piecetype(bp.get_piece()) == Pawn 
            && (
                (us == White && bp.get_square() == board.get_ep_square() + S) 
                || (us == Black && bp.get_square() == board.get_ep_square() + N)
            );

        if(board_pieces[i].get_square() == from) 
            board_pieces[i].update(to, square_w, player_colour);

        if(capture || en_passant)
            board_pieces[i].remove();
        else if(black_castle_kingside_rook) 
            board_pieces[i].update(f8, square_w, player_colour);
        else if(white_castle_kingside_rook)
            board_pieces[i].update(f1, square_w, player_colour);
        else if(black_castle_queenside_rook)
            board_pieces[i].update(f8, square_w, player_colour);
        else if(white_castle_queenside_rook)
            board_pieces[i].update(f1, square_w, player_colour);
        else if(promotion && bp.get_square() == from) {
            printf("Promoting!\n");
            PieceType promotion_pt = promotion_to_piecetype(flag);
            Piece promotion = piecetype_to_piece(promotion_pt, us);
            board_pieces[i].promote(promotion, board_renderer);
        } 
    }
    printf("Engine make move: %s%s\n", square_to_str(from).c_str(), square_to_str(to).c_str());
    board.make_move(reply);
    board.print_squares();
}

void PlayerView::player_make_move()
{
    if(current_move.from == current_move.to)
        return;

    printf("moved %s%s\n", square_to_str(current_move.from).c_str(), square_to_str(current_move.to).c_str());

    Colour us = board.get_side_to_move();
    Move flag = QUIET;
    // Sketchy castles
    bool moving_white_king = us == White && current_move.from == e1 && board.get_piece(current_move.from) == WK;
    bool moving_black_king = us == Black && current_move.from == e8 && board.get_piece(current_move.from) == BK;
    if(moving_white_king || moving_black_king) {
        bool moving_to_g1 = current_move.to == g1 && board.get_piece(h1) == WR;
        bool moving_to_g8 = current_move.to == g8 && board.get_piece(h8) == BR;
        bool moving_to_c1 = current_move.to == c1 && board.get_piece(a1) == WR;
        bool moving_to_c8 = current_move.to == c8 && board.get_piece(a8) == BR;

        if(moving_to_g1 || moving_to_g8)
            flag = OO;
        else if(moving_to_c1 || moving_to_c8)
            flag = OOO;
    }

    bool pawn_adjacent_from = 
        (board.get_op_piece_bb(Pawn) & bit(current_move.from + W) & ~FileHBB) ||
        (board.get_op_piece_bb(Pawn) & bit(current_move.from + E) & ~FileABB);

    bool pawn_above_to = (
        us == White && board.get_piece(current_move.to + S) == BP 
        || us == Black && board.get_piece(current_move.to + N) == WP
    );

    // Extremely loose en passant
    bool en_passant = pawn_above_to && pawn_adjacent_from;

    bool promotion = piece_to_piecetype(board.get_piece(current_move.from)) == Pawn && on_opposite_rank(current_move.to, us);

    for(size_t i = 0; i < 32; ++i) {
        BoardPiece bp = board_pieces[i];
        Piece p = bp.get_piece();

        bool pawn_on_ep_square = 
            (us == Black && p == WP && (bp.get_square() + S) == board.get_ep_square())
         || (us == White && p == BP && (bp.get_square() + N) == board.get_ep_square());

        // Capture
        if(p != None && bp.get_square() == current_move.to && board.get_piece_colour(p) != us)
            board_pieces[i].remove();
        else if(us == White && flag == OO && bp.get_square() == h1 && bp.get_piece() == WR) // Move white rook
            board_pieces[i].update(f1, square_w, player_colour);
        else if(us == White && flag == OOO && bp.get_square() == a1 && bp.get_piece() == WR) // Move white rook
            board_pieces[i].update(d1, square_w, player_colour);
        else if(us == Black && flag == OO && bp.get_square() == h8 && bp.get_piece() == BR) // Move white rook
            board_pieces[i].update(f8, square_w, player_colour);
        else if(us == Black && flag == OOO && bp.get_square() == a8 && bp.get_piece() == BR) // Move white rook
            board_pieces[i].update(d8, square_w, player_colour);
        else if(en_passant && pawn_on_ep_square) {
            board_pieces[i].remove();
            flag = EN_PASSANT;
        } else if(promotion && piece_to_piecetype(bp.get_piece()) == Pawn && on_opposite_rank(bp.get_square(), us)) {
            board_pieces[i].promote(piecetype_to_piece(Queen, us), board_renderer);
            flag = PROMOTE_QUEEN;
        }
    }

    BMove m = move(current_move.from, current_move.to, flag);
    board.make_move(m);
    board.print_squares();
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
            } else if(e.type == SDL_KEYUP) {
                if(e.key.keysym.sym == SDLK_e) {
                    editing = !editing;
                    if(editing)
                        printf("No engine. board inverted? %s\n", board_inverted ? "Yes" : "No");
                    else   
                        printf("Engine playing %s. board inverted? %s\n", colour_to_str(!player_colour).c_str(), board_inverted ? "Yes" : "No");
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

        uint64_t end = SDL_GetPerformanceCounter();

        float elapsed = (end - start) / (float)(SDL_GetPerformanceFrequency() * 1000.0f);

        // Cap frames
        // if(!piece_being_dragged)
        // unsigned int ticks = SDL_GetTicks(); 
        // if(ticks < SCREEN_TICKS_PER_FRAME)
        //     SDL_Delay(SCREEN_TICKS_PER_FRAME - ticks);

        if( ((board_inverted && board.get_side_to_move() == player_colour) || (!board_inverted && board.get_side_to_move() != player_colour)) && !editing)
            engine_make_move();
    }

    SDL_DestroyRenderer(board_renderer);
    SDL_DestroyWindow(board_window);
     
    // Close SDL 
    SDL_Quit(); 
}
