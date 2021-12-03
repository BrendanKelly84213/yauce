#include "PlayerView.h"
        
void PlayerView::init_all()
{
    board_state.init(fen);
    init_window_and_renderer();
    init_viewports();
    pieces_texture.loadFromFile("../assets/ChessPiecesArray.png", renderer);
    init_piece_clips();
    init_pieces();
    TTF_Init();
    font = TTF_OpenFont("../assets/OpenSans-Regular.ttf", 16);
    update_moves();
    init_generator();
}

void PlayerView::init_piece_clips() 
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

bool PlayerView::update_info()
{
    SDL_Color white = { 255, 255, 255 }; 

    std::string side_to_move_ch = (board_state.side_to_move == White) ? " White " : " Black ";
    std::string side_to_move_str = "side to move: "; 
    std::string side_to_move = side_to_move_str + side_to_move_ch;
    if(!side_to_move_texture.loadText(font, side_to_move, white, renderer)) {
       return false;
    }

    std::string can_w_castle_ks = board_state.w_castle_ks ? " Yes " : " No ";
    std::string w_castle_ks_str = "white castle kingside: ";
    std::string w_castle_ks = w_castle_ks_str + can_w_castle_ks;
    if(!w_castling_ks_texture.loadText(font, w_castle_ks, white, renderer)) {
        return false;
    }

    std::string can_w_castle_qs = board_state.w_castle_qs ? " Yes " : " No ";
    std::string w_castle_qs_str = "white castle queenside: ";
    std::string w_castle_qs = w_castle_qs_str + can_w_castle_qs;
    if(!w_castling_qs_texture.loadText(font, w_castle_qs, white, renderer)) {
        return false;
    }
    
    std::string can_b_castle_ks = board_state.b_castle_ks ? " Yes " : " No ";
    std::string b_castle_ks_str = "black castle kingside: ";
    std::string b_castle_ks = b_castle_ks_str + can_b_castle_ks;
    if(!b_castling_ks_texture.loadText(font, b_castle_ks, white, renderer)) {
        return false;
    }

    std::string can_b_castle_qs = board_state.b_castle_qs ? " Yes " : " No ";
    std::string b_castle_qs_str = "black castle queenside: ";
    std::string b_castle_qs = b_castle_qs_str + can_b_castle_qs;
    if(!b_castling_qs_texture.loadText(font, b_castle_qs, white, renderer)) {
        return false;
    }

    std::string ep_file_str = "ep file: ";
    std::string ep_file_data = board_state.ep_file > 0 ? std::to_string(board_state.ep_file) : "None";
    std::string ep_file = ep_file_str + ep_file_data;
    if(!ep_file_texture.loadText(font, ep_file, white, renderer)) {
        return false;
    }

    std::string halfmove_clock_str = "halfmove clock: "; 
    std::string halfmove_clock_data = std::to_string(board_state.halfmove_clock);
    std::string halfmove_clock = halfmove_clock_str + halfmove_clock_data;
    if(!halfmove_clock_texture.loadText(font, halfmove_clock, white, renderer)) {
        return false;
    }

    std::string ply_count_str = "ply count: "; 
    std::string ply_count_data = std::to_string(board_state.ply_count);
    std::string ply_count = ply_count_str + ply_count_data;
    if(!ply_count_texture.loadText(font, ply_count, white, renderer)) {
        return false;
    }

    return true;
}

bool PlayerView::valid_move(Square from, Square to)
{ 
    for(auto &m : move_vec) {
        if(m.from == from && m.to == to
            && m.from != m.to) {
            return true;
        }
    }
    return false;
}

// TODO: Bad place for this
Colour operator!(Colour c)
{
    return static_cast<Colour>(!static_cast<bool>(c));
}

inline bool did_pawn_double_push(Colour side, Square from, Square to)
{
    if(side == White)
        return from >= a2 && from <= h2 && to == from + N + N; 
    return from >= a7 && from <= h7 && to == from + S + S; 
}

inline bool correct_colour(Colour side, Square s)
{
    if(side == White)
        return s >= a2 && s <= h2 ; 
    return s >= a7 && s <= h7; 
}

void PlayerView::on_player_make_move(Piece piece)
{
    // Update game state  
    if(did_pawn_double_push(board_state.side_to_move, current_move.from, current_move.to)) {
        board_state.ep_file = current_move.to % 8;
    } else {
        board_state.ep_file = -1;
    }

    // TODO: castling rights, halfmove_clock
    board_state.ply_count++;
    board_state.side_to_move = !board_state.side_to_move;
    board_state.squares[current_move.from] = None;
    board_state.squares[current_move.to] = piece;
    current_move.from = current_move.to;
    update_moves();
}

Square xy_to_square(int x, int y, int rect_w, int rect_h)
{
    int file = ((x + rect_w/2) / rect_w);
    int rank = 7-((y + rect_w/2) / rect_w);
    return static_cast<Square>(8*rank + file);
}

void PlayerView::handle_events()
{
    switch(e.type) {
        case SDL_KEYDOWN : 
            if(e.key.keysym.sym == SDLK_q) {
                running = false;
            }
            break; 
        case SDL_MOUSEBUTTONUP :
            mousedown_on_piece = false;
            for(int s=0; s<NUM_SQUARES; ++s) {
                if(board_pieces[s].dragging && board_pieces[s].p != -1) {
                    Square from = current_move.from;
                    Square to = xy_to_square(board_pieces[s].x, board_pieces[s].y, rect_w, rect_w);
                    current_move.to = to;
                    
                    if(valid_move(from, to)) {
                        snap(board_pieces[s]);
                        on_player_make_move(board_pieces[s].p);
                    } else {
                        board_pieces[s].x = rect_w*(from % 8);
                        board_pieces[s].y = rect_w*(floor(7-(from / 8)));
                    }
                }
            }
            break;
        case SDL_MOUSEBUTTONDOWN :
            for(int s=0; s<NUM_SQUARES; ++s) {
                if(in_board_piece(board_pieces[s]) && board_pieces[s].p != -1) {
                    mousedown_on_piece = true;
                    Square sq = xy_to_square(board_pieces[s].x, board_pieces[s].y, rect_w, rect_w);
                    // Correct colour
                    if(correct_colour) {
                        current_move.from = sq;
                        /* std::cout << current_move.from << '\n'; */
                    }
                }
            }
            break;
        default : break;
    }
}

void PlayerView::draw_grid()
{
    for(int i=0; i<NUM_ROWS; ++i) {
        for(int j=0; j<NUM_ROWS; ++j){
            SDL_Rect square;
            square.w = rect_w;
            square.h = rect_w;
            square.x = i*rect_w;
            square.y = j*rect_w;

            if(((i+j)%2)){ 
                SDL_SetRenderDrawColor(renderer, 200,200,255,0);
            } else {
                SDL_SetRenderDrawColor(renderer, 15,50,20,0);
            }
            SDL_RenderFillRect(renderer, &square);
        }
    }
}

void PlayerView::init_pieces()
{
    for(int s=0; s<NUM_SQUARES; ++s) {
        Piece p = board_state.squares[s];
        if(p != None) {
            board_pieces[s].p = p;
            board_pieces[s].x = (s % 8)*rect_w;
            board_pieces[s].y = (7 - (s / 8))*rect_w;
            board_pieces[s].w = rect_w;
            board_pieces[s].h = rect_w;
        }
    }
}

void PlayerView::draw_pieces() 
{
    for(int s=0; s<NUM_SQUARES; ++s) {
        Piece p = board_pieces[s].p;
        int x = board_pieces[s].x;
        int y = board_pieces[s].y;
        if(p != None)
            pieces_texture.render(x, y, rect_w, rect_w, renderer, &piece_clips[p]);
    }
}

void PlayerView::draw_info()
{
    side_to_move_texture.render(10,0, renderer);
    w_castling_ks_texture.render(10, 30, renderer);
    w_castling_qs_texture.render(10, 60, renderer);
    b_castling_ks_texture.render(10, 90, renderer);
    b_castling_qs_texture.render(10, 120, renderer);
    ep_file_texture.render(10, 150, renderer);
    halfmove_clock_texture.render(10, 180, renderer);
    ply_count_texture.render(10, 210, renderer);
}

void PlayerView::game_loop()
{
	while(running) {
		if(SDL_PollEvent(&e)) {
            handle_events();
		}

        SDL_GetMouseState(&mx, &my);
		SDL_GetWindowSize(window, &scr_w, &scr_w);

		// Clear screen
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);

        SDL_RenderSetViewport(renderer, &board_viewport);
        draw_grid();
        draw_pieces();
        handle_dragging();

        SDL_RenderSetViewport(renderer, &info_viewport);
        update_info();
        draw_info();

		SDL_RenderPresent(renderer);
	}
    close_sdl();
    free_textures();
}

int PlayerView::init_window_and_renderer()
{
    return init(&window, &renderer);
}

void PlayerView::close_sdl() 
{
	close(&window, &renderer);
}

bool PlayerView::no_others_dragged(int this_board_piece_idx) 
{
    for(int i=0; i<NUM_SQUARES; ++i) {
        if(board_pieces[i].p != -1 
                && board_pieces[i].dragging 
                && i != this_board_piece_idx) {
            return false;
        } 
    }
    return true;
}

bool PlayerView::in_board_piece(BoardPiece todrag) 
{
    return mx >= todrag.x 
            && mx <= todrag.x + todrag.w
            && my >= todrag.y 
            && my <= todrag.y + todrag.h ;
}

void PlayerView::snap(BoardPiece & piece)
{
    int file = (piece.x + piece.w/2) / rect_w;
    int rank = 8-((piece.y + piece.h/2)/ rect_w);
    piece.x = file*rect_w;
    piece.y = (8-rank)*rect_w;
}

void PlayerView::handle_dragging() 
{
    for(int s=0; s<NUM_SQUARES; ++s) {
        if ( board_pieces[s].p != None
            && mousedown_on_piece
            && in_board_piece(board_pieces[s])
            && no_others_dragged(s) ) {
            board_pieces[s].dragging = true;
        }

        if(!mousedown_on_piece) {
            board_pieces[s].dragging = false;
        }

        if(board_pieces[s].dragging && mx < board_viewport.w) {
            int cx = board_pieces[s].w / 2;
            int cy = board_pieces[s].h / 2;

            board_pieces[s].x = mx - cx;
            board_pieces[s].y = my - cy; 
        }
    }
}

void PlayerView::init_viewports()
{
    // board viewport
    board_viewport.x = 0;
    board_viewport.y = 0;
    board_viewport.w = (0.6666)*scr_w;
    board_viewport.h = scr_h;

    // info viewport
    info_viewport.x = board_viewport.w; 
    info_viewport.y = 0; 
    info_viewport.w = scr_w/3;
    info_viewport.h = scr_h;

    rect_w = board_viewport.w / NUM_ROWS;
}

void PlayerView::free_textures()
{
    side_to_move_texture.free();
    w_castling_ks_texture.free();
    w_castling_qs_texture.free();
    b_castling_ks_texture.free();
    b_castling_qs_texture.free();
    ep_file_texture.free();
    halfmove_clock_texture.free();
    ply_count_texture.free();
}

void PlayerView::update_moves()
{
    move_vec = boardstate_to_move_vec(board_state);
}

