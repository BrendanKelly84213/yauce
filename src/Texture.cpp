#include "Texture.h"
    
Texture::~Texture()
{
    free();
}

//Make an image into an SDL_Texture
bool Texture::loadFromFile( std::string path, SDL_Renderer* renderer )
{
	free();
	
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL ) {
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
		return NULL;
	} 

	//Color key image
	SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

	//Create texture from surface pixels
	newTexture = SDL_CreateTextureFromSurface( renderer, loadedSurface );
	if( newTexture == NULL ) {
		printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		return NULL;
	}

	//Get rid of old loaded surface
	SDL_FreeSurface( loadedSurface );

	texture = newTexture;
    return texture != NULL;
}

bool Texture::loadText(TTF_Font * font, std::string text, SDL_Color color, SDL_Renderer* renderer)
{
    free();
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if(surface == NULL) {
        std::cerr << " Unable to render text: " << TTF_GetError() << '\n';
        return false;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if(texture == NULL) {
        std::cerr << "Unable to create texture from surface: " << TTF_GetError() << '\n';
        return false;
    }

    w = surface->w;
    h = surface->h;
    
    SDL_FreeSurface(surface);
    return texture != NULL;
}

void Texture::render(int x, int y, int dst_w, int dst_h, SDL_Renderer* renderer, SDL_Rect* clip) 
{   
    SDL_Rect rect = { x, y, w, h };

    if(clip != NULL) {
        rect.w = dst_w;
        rect.h = dst_h;
    }

    SDL_RenderCopy(renderer, texture, clip, &rect);
}

void Texture::render(int x, int y, SDL_Renderer* renderer, SDL_Rect* clip) 
{   
    SDL_Rect rect = { x, y, w, h };

    if(clip != NULL) {
        rect.w = clip->w;
        rect.h = clip->h;
    }

    SDL_RenderCopy(renderer, texture, clip, &rect);
}

void Texture::free() 
{
	if(texture != NULL) {
		SDL_DestroyTexture(texture);
		texture = NULL;
	}
}
