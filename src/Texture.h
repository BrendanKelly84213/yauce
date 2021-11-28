#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

class Texture
{
public:
    Texture() : texture(NULL), w(0), h(0) {}
    ~Texture();
    bool loadFromFile(std::string path, SDL_Renderer* renderer);
    bool loadText(TTF_Font* font, std::string text, SDL_Color color, SDL_Renderer* renderer);
    void render(int x, int y, int dst_w, int dst_h, SDL_Renderer* renderer, SDL_Rect* clip = NULL);
    void render(int x, int y, SDL_Renderer* renderer, SDL_Rect* clip = NULL);
    void free();
private:
    SDL_Texture* texture;
    int w, h;
};

#endif
