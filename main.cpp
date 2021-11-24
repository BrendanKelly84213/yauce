#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <iostream>

#include "environment.h"
/* #include "drag.h" */

//TODO: 
// Pieces should have reference to what square they're on  

const int NUM_PIECES = 16;
const int MAX_PIECES = 32;
const int NUM_ROWS = 8;

typedef std::string Fen; 

enum Piece { 
    WQ, WK, WR, WN, WB, WP,
    BQ, BK, BR, BN, BB, BP, None=-1
};

enum Rank { a, b, c, d, e, f, g, h};

enum Square : int
{
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2, 
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4, 
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

enum Colour { White, Black };

struct BoardPiece 
{
    SDL_Rect rect;
    bool dragging = false; 
    Square s;
    Piece p = None;
};

struct BoardState
{
    BoardPiece piece_placement[64];
    Colour side_to_move; 
    bool w_castle_ks = false; 
    bool w_castle_qs = false; 
    bool b_castle_ks = false; 
    bool b_castle_qs = false; 
    int ep_file;
    int ply_count; 
};

bool no_others_dragged(BoardPiece * board_pieces, int num_board_pieces, int this_board_piece_idx) 
{
    for(int i=0; i<num_board_pieces; ++i) {
        if(board_pieces[i].dragging && i != this_board_piece_idx) {
            return false;
        } 
    }
    return true;
}

bool in_board_piece(BoardPiece todrag, int mx, int my) 
{
    return mx >= todrag.rect.x 
            && mx <= todrag.rect.x + todrag.rect.w
            && my >= todrag.rect.y 
            && my <= todrag.rect.y + todrag.rect.h ;
}

void snap(BoardPiece & piece, int rect_w)
{
    int file = (piece.rect.x + piece.rect.w/2) / rect_w;
    int rank = 8-((piece.rect.y + piece.rect.h/2)/ rect_w);
    piece.rect.x = file*rect_w;
    piece.rect.y = (8-rank)*rect_w;
}

// Handle dragging 
void handle_dragging(bool mousedown, int mx, int my, BoardPiece * todrag, int num_board_pieces) 
{

    for(int i=0; i<num_board_pieces; ++i) {
        if ( mousedown
            && in_board_piece(todrag[i], mx, my)
            && no_others_dragged(todrag, num_board_pieces, i) ) {
            todrag[i].dragging = true;
        }

        if( !mousedown ) {
            todrag[i].dragging = false;
        }

        if(todrag[i].dragging) {
            int cx = todrag[i].rect.w / 2;
            int cy = todrag[i].rect.h / 2;

            todrag[i].rect.x = mx - cx;
            todrag[i].rect.y = my - cy; 
        }
    }
}

void init_piece_clips(SDL_Rect * piece_clips) 
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

bool is_piece_ch(char ch) 
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

Piece fen_to_piece(char ch) 
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
    
char piece_to_char(Piece p) 
{
    switch(p) {
        case BQ:
            return 'q';
        case BK:
            return 'k';
        case BR:
            return 'r';
        case BN:
            return 'n';
        case BB:
            return 'b';
        case BP:
            return 'p';

        case WQ:
            return 'Q';
        case WK:
            return 'K';
        case WR:
            return 'R';
        case WN:
            return 'N';
        case WB:
            return 'B';
        case WP:
            return 'P';

        default: break;

    }
    return ' ';
}

void init_pieces(BoardPiece * board_pieces, int rect_w, Fen fen_str) 
{
    // Parse out pieces 
    int rank=8;
    int file=0;
    int i=0;
    int j=0;

    while(fen_str[i] != ' ') {
        int sq = rank * file;

        if(fen_str[i] == '/') {
            rank--;
            file=0;
        }

        if(fen_str[i] >= '0' && fen_str[i] <= '8') {
            file += (fen_str[i] - '0');
        }

        if(is_piece_ch(fen_str[i])) {
            const Piece piece = fen_to_piece(fen_str[i]);

            board_pieces[j].p = piece;
            board_pieces[j].rect.w = rect_w;
            board_pieces[j].rect.h = rect_w;
            
            board_pieces[j].rect.x = (file)*rect_w ;
            board_pieces[j].rect.y = (8-rank)*rect_w ;

            file++;
            j++;
        }
        i++;
    }
}

int main( int argc, char *argv[] )
{
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* texture = NULL;
    SDL_Event e;
    SDL_Rect piece_clips[NUM_PIECES];
    BoardPiece board_pieces[MAX_PIECES]; 

    int scr_w = 640;
    int rect_w = scr_w / NUM_ROWS;
    bool running = true;
    bool mousedown_on_piece;
    int mx, my;

    Fen fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";

	//Initialize piece clippings array 
    init_piece_clips(piece_clips); 

	SDL_PollEvent( &e );
	SDL_GetWindowSize( window, &scr_w, &scr_w );

	if(!init(&window, &renderer)) {
		return 1;
	}

    // Pieces as they appear on the board 
    init_pieces(board_pieces, rect_w, fen);

	// Load image from file 
	texture = loadFromFile("ChessPiecesArray.png", renderer, texture);

	while( running ) {
		if( SDL_PollEvent( &e ) ) {
            switch( e.type ) {
                case SDL_KEYDOWN : 
                    if( e.key.keysym.sym == SDLK_q ) {
                        running = false;
                    }
                    break; 
                case SDL_MOUSEBUTTONUP :
                    mousedown_on_piece = false;
                    for(int i=0; i<MAX_PIECES; ++i) {
                        if(board_pieces[i].dragging)
                            snap(board_pieces[i], rect_w);
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN :
                    for(int i=0; i<MAX_PIECES; ++i) {
                        if(in_board_piece(board_pieces[i], mx, my)) {
                            mousedown_on_piece = true;
                        }
                    }
                    break;
                default : break;
            }
		}

        SDL_GetMouseState(&mx, &my);

		// Clear screen
		SDL_SetRenderDrawColor( renderer, 0, 0, 0, SDL_ALPHA_OPAQUE );
		SDL_RenderClear( renderer );

        // Draw grid 
		bool isw = false;
		for(int i=0; i<NUM_ROWS; ++i) {
			for(int j=0; j<NUM_ROWS; ++j){
				const int sq_num = j*NUM_ROWS + i;
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

		// Update window size
		SDL_GetWindowSize( window, &scr_w, &scr_w);
        
        // Draw pieces
        for(int i=0; i<MAX_PIECES; ++i) {
            const Piece p = board_pieces[i].p;
            SDL_RenderCopy(renderer, texture, &piece_clips[p], &board_pieces[i].rect);	

        }

        handle_dragging(mousedown_on_piece, mx, my, board_pieces, MAX_PIECES);

		//Update screen
		SDL_RenderPresent( renderer );
	}

	//Free and close SDL
	close( & window, & renderer );

	return 0;
}
