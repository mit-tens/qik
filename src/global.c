#include <stdbool.h>
#include <SDL2/SDL.h>
#include <libconfig.h>
#include "qik/types.h"
#include "global.h"

bool running;

SDL_Renderer *gRenderer;
SDL_Window *gWindow;
Uint32 gFormat;

double time, oldtime, frametime;

Bind gBind;
SDL_Surface **gSurface;
Sprite *gSprite;
Config gConfig;
Map gMap;
User gUser;

config_t gconfig_t, mconfig_t;
