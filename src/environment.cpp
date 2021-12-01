#include "environment.h"

bool init( SDL_Window** window, SDL_Renderer** renderer )
{
	if( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
		printf( "Failed to initialize video %s\n", SDL_GetError() );
		return false;
	} 

	*window = SDL_CreateWindow( 
			"Window", 
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH,
			SCREEN_HEIGHT,
			0
	);
	if( *window == NULL ) {
		printf( "Could not create window: %s\n", SDL_GetError() );
		return false;	
	} 

	/* *renderer = SDL_CreateRenderer( *window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); */
	*renderer = SDL_CreateRenderer( *window, -1, SDL_RENDERER_ACCELERATED );
	if( *renderer == NULL ) {
		printf( "Could not create renderer: %s\n" );
		return false;
	}
	SDL_SetRenderDrawColor( *renderer, 0xFF, 0xFF, 0xFF, 0xFF );

	int imgFlags = IMG_INIT_PNG;
	if( !( IMG_Init( imgFlags ) & imgFlags ) ) {
		printf( "SDL_image could not initialize! %s\n", IMG_GetError() );
		return false;
	}

	return true;
}

void close(SDL_Window** window, SDL_Renderer** renderer)
{          
	SDL_DestroyWindow( *window );
	*window = NULL;

	SDL_DestroyRenderer( *renderer );
	*renderer = NULL;
	
	IMG_Quit();
	SDL_Quit();
    TTF_Quit();
}


