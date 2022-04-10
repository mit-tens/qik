#ifndef GLOBAL_H
#define GLOBAL_H

extern bool running;

extern SDL_Renderer *gRenderer;
extern SDL_Window *gWindow;
extern Uint32 gFormat;

extern double time, oldtime, frametime;

extern Bind gBind;
extern SDL_Surface **gSurface;
extern Sprite *gSprite;
extern Config gConfig;
extern Map gMap;
extern User gUser;

extern config_t gconfig_t, mconfig_t;

#endif
