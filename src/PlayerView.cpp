#include "PlayerView.h"
        
void PlayerView::init_all()
{
    board_state.init(fen);
    init_piece_clips();
    init_pieces();
    init_window_and_renderer();
    init_texture();
}

void PlayerView::init_pieces()
{
    for(int r=0; r<NUM_ROWS; ++r) {
        for(int f=0; f<NUM_ROWS; ++f) {
            const int s = r*8 + f;
            const Piece p = board_state.squares[s];

            board_pieces[s].p = p;
            board_pieces[s].rect.w = rect_w; 
            board_pieces[s].rect.h = rect_w; 
            board_pieces[s].rect.x = f*rect_w; 
            board_pieces[s].rect.y = r*rect_w; 
        }
    }
}

void PlayerView::init_piece_clips() 
{
    for(int i=0; i<NUM_PIECES; ++i) {
        piece_clips[i].w = 60;
        piece_clips[i].h = 60;
    }

    piece_clips[BQ].x = 0;
    piece_clips[BQ].y = 0;

    piece_clips[BK].x = 60;
    piece_clips[BK].y = 0;

    piece_clips[BR].x = 120;
    piece_clips[BR].y = 0;

    piece_clips[BN].x = 180;
    piece_clips[BN].y = 0;

    piece_clips[BB].x = 240;
    piece_clips[BB].y = 0;

    piece_clips[BP].x = 300;
    piece_clips[BP].y = 0;


    piece_clips[WQ].x = 0;
    piece_clips[WQ].y = 60;

    piece_clips[WK].x = 60;
    piece_clips[WK].y = 60;

    piece_clips[WR].x = 120;
    piece_clips[WR].y = 60;

    piece_clips[WN].x = 180;
    piece_clips[WN].y = 60;

    piece_clips[WB].x = 240;
    piece_clips[WB].y = 60;

    piece_clips[WP].x = 300;
    piece_clips[WP].y = 60;
}

void PlayerView::handle_events()
{
    switch( e.type ) {
        case SDL_KEYDOWN : 
            if( e.key.keysym.sym == SDLK_q ) {
                running = false;
            }
            break; 
        case SDL_MOUSEBUTTONUP :
            mousedown_on_piece = false;
            for(int i=0; i<NUM_SQUARES; ++i) {
                if(board_pieces[i].dragging && board_pieces[i].p != -1) {
                    snap(board_pieces[i]);
                }
            }
            break;
        case SDL_MOUSEBUTTONDOWN :
            for(int i=0; i<NUM_SQUARES; ++i) {
                if(in_board_piece(board_pieces[i]) && board_pieces[i].p != -1) {
                    mousedown_on_piece = true;
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

void PlayerView::draw_pieces() 
{
    for(int i=0; i<NUM_SQUARES; ++i) {
        const Piece p = board_pieces[i].p;
        if( p != None ) 
            SDL_RenderCopy(renderer, texture, &piece_clips[p], & board_pieces[i].rect);	
    }
}

void PlayerView::game_loop()
{
	while(running) {
		if(SDL_PollEvent(&e)) {
            handle_events();
		}

        SDL_GetMouseState(&mx, &my);

		// Clear screen
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		SDL_GetWindowSize(window, &scr_w, &scr_w);

        draw_grid();
        draw_pieces();
        handle_dragging();

		SDL_RenderPresent(renderer);
	}
}

void PlayerView::init_texture()
{
	texture = loadFromFile("../assets/ChessPiecesArray.png", renderer, texture);
}

int PlayerView::init_window_and_renderer()
{
    return init(&window, &renderer);
}

void PlayerView::close_sdl() 
{
	close(&window,&renderer );
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
    return mx >= todrag.rect.x 
            && mx <= todrag.rect.x + todrag.rect.w
            && my >= todrag.rect.y 
            && my <= todrag.rect.y + todrag.rect.h ;
}

void PlayerView::snap(BoardPiece & piece)
{
    int file = (piece.rect.x + piece.rect.w/2) / rect_w;
    int rank = 8-((piece.rect.y + piece.rect.h/2)/ rect_w);
    piece.rect.x = file*rect_w;
    piece.rect.y = (8-rank)*rect_w;
}

void PlayerView::handle_dragging() 
{

    for(int i=0; i<NUM_SQUARES; ++i) {
        if ( board_pieces[i].p != -1
            && mousedown_on_piece
            && in_board_piece(board_pieces[i])
            && no_others_dragged(i) ) {
            board_pieces[i].dragging = true;
        }

        if(!mousedown_on_piece) {
            board_pieces[i].dragging = false;
        }

        if(board_pieces[i].dragging) {
            int cx = board_pieces[i].rect.w / 2;
            int cy = board_pieces[i].rect.h / 2;

            board_pieces[i].rect.x = mx - cx;
            board_pieces[i].rect.y = my - cy; 
        }
    }
}


