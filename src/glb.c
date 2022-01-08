#include <stdbool.h>
#include <SDL2/SDL.h>
#include <libconfig.h>
#include "types.h"
#include "glb.h"

bool glb_running;

SDL_Renderer *glb_renderer;
SDL_Window *glb_window;
Uint32 glb_format;

float glb_time, glb_time_old, glb_time_frame;

qik_bind glb_bind;
qik_mtex glb_mtex;
qik_mspr glb_mspr;
qik_cfg glb_cfg;
qik_map glb_map;
qik_usr glb_usr;

config_t glb_cfg_cfg_t, glb_map_cfg_t;
