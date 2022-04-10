#ifndef MAP_H
#define MAP_H

int
load_map(Map *, User *, SDL_Surface **, Sprite *, config_t *, char *, Uint32);

void
unload_map(Map *, User *, SDL_Surface **, Sprite *, config_t *);

#endif
