#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>

#include "../BoardState.h"
#include "../Generator.h"
#include "../utils/conversions.h"
#include "../utils/types.h"

namespace playerview {

bool init(std::string fen);
void run();

}


#endif
