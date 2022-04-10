#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "image.h"

SDL_Surface*
load_surface(char *path, Uint32 format)
{
    SDL_Surface *loaded_surface = SDL_LoadBMP(path);

    SDL_Surface *opt_surface = SDL_ConvertSurfaceFormat(loaded_surface, format, 0);
    
    SDL_FreeSurface(loaded_surface);

    return opt_surface;
}