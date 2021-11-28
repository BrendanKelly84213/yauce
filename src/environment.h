#ifndef ENVIRONEMNT_H
#define ENVIRONEMNT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>

const int SCREEN_WIDTH = (1.5)*640 ;
const int SCREEN_HEIGHT = 640;

bool init(SDL_Window** window, SDL_Renderer** renderer);
void close(SDL_Window** window, SDL_Renderer** renderer);

#endif
