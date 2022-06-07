#include "BoardPiece.h"
#include "helpers.h"
#include <SDL2/SDL_image.h>


void BoardPiece::update(int x, int y, int w)
{
    rect.w = w; 
    rect.h = w; 
    rect.x = x;
    rect.y = y;

    int center_x = x + (0.5 * w);
    int center_y = y + (0.5 * w);
    size_t rank = y_to_rank(center_y, w);
    size_t file = x_to_file(center_x, w);
    s = square(rank, file);
}

void BoardPiece::update(int w)
{
    int x = square_to_x(s, w);
    int y = square_to_y(s, w);
    rect.w = w;
    rect.h = w;
    rect.x = x;
    rect.y = y;
}

void BoardPiece::init(Piece _p, Square _s, int w, SDL_Renderer * renderer)
{
    int x = square_to_x(s, w);
    int y = square_to_y(s, w);
    p = _p;
    s = _s;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = w;
    make_piece_texture(renderer);
}

void BoardPiece::make_piece_texture(SDL_Renderer * renderer)
{
    std::string path = svg_path[p];
    printf("Got path: %s\n", path.c_str());
    SDL_Surface * surface = IMG_Load(path.c_str());
    if(surface == NULL) {
        printf("Error getting surface from svg: %s, %s\n", path.c_str(), SDL_GetError());
        exit(1);
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if(texture == NULL) {
        printf("Error creating texture from surface: %s\n", SDL_GetError());
        exit(1);
    }
}