#ifndef ENVIRONEMNT_H
#define ENVIRONEMNT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;

void free( SDL_Texture** texture );
//Make an image into an SDL_Texture
SDL_Texture* loadFromFile( std::string path, 
		SDL_Renderer* renderer, SDL_Texture* prevTexture );

bool init( SDL_Window** window, SDL_Renderer** renderer );
void close( SDL_Window** window, SDL_Renderer** renderer, SDL_Texture** texture=NULL );

#endif
