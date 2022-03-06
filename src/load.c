#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "load.h"

SDL_Surface*
load_surface(char *path, Uint32 format)
{
    SDL_Surface *loaded_surface = SDL_LoadBMP(path);

    if (loaded_surface == NULL)
        fprintf(stderr, "In load_surface(): %s %s\n", path, SDL_GetError());

    SDL_Surface *opt_surface = SDL_ConvertSurfaceFormat(loaded_surface, format, 0);

    if (opt_surface == NULL)
        fprintf(stderr, "In load_surface(): %s %s\n", path, SDL_GetError());
    
    SDL_FreeSurface(loaded_surface);

    return opt_surface;
}

SDL_Texture*
load_texture(SDL_Renderer *renderer, char *path, Uint32 format)
{
    SDL_Texture *loaded_texture = SDL_CreateTextureFromSurface(renderer, load_surface(path, format));

    if (loaded_texture == NULL)
        fprintf(stderr, "In load_texture(): %s %s\n", path, SDL_GetError());

    return loaded_texture;
}
