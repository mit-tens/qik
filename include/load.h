#ifndef LOAD_H
#define LOAD_H

SDL_Surface*
load_surface(const char *path, const Uint32 format);

SDL_Texture*
load_texture(SDL_Renderer *renderer, const char *path, const Uint32 format);

#endif
