#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <libconfig.h>
#include "types.h"
#include "glb.h"
#include "common.h"
#include "bind.h"

typedef struct Key Key;
struct Key {
    char *t;
    SDL_Scancode s;
};

static Key Keylist[] =
{
    {"A", SDL_SCANCODE_A},
    {"B", SDL_SCANCODE_B},
    {"C", SDL_SCANCODE_C},
    {"D", SDL_SCANCODE_D},
    {"E", SDL_SCANCODE_E},
    {"F", SDL_SCANCODE_F},
    {"G", SDL_SCANCODE_G},
    {"H", SDL_SCANCODE_H},
    {"I", SDL_SCANCODE_I},
    {"J", SDL_SCANCODE_J},
    {"K", SDL_SCANCODE_K},
    {"L", SDL_SCANCODE_L},
    {"M", SDL_SCANCODE_M},
    {"N", SDL_SCANCODE_N},
    {"O", SDL_SCANCODE_O},
    {"P", SDL_SCANCODE_P},
    {"Q", SDL_SCANCODE_Q},
    {"R", SDL_SCANCODE_R},
    {"S", SDL_SCANCODE_S},
    {"T", SDL_SCANCODE_T},
    {"U", SDL_SCANCODE_U},
    {"V", SDL_SCANCODE_V},
    {"W", SDL_SCANCODE_W},
    {"X", SDL_SCANCODE_X},
    {"Y", SDL_SCANCODE_Y},
    {"Z", SDL_SCANCODE_Z},
    {"0", SDL_SCANCODE_0},
    {"1", SDL_SCANCODE_1},
    {"2", SDL_SCANCODE_2},
    {"3", SDL_SCANCODE_3},
    {"4", SDL_SCANCODE_4},
    {"5", SDL_SCANCODE_5},
    {"6", SDL_SCANCODE_6},
    {"7", SDL_SCANCODE_7},
    {"8", SDL_SCANCODE_8},
    {"9", SDL_SCANCODE_9},
    {"F1", SDL_SCANCODE_F1},
    {"F2", SDL_SCANCODE_F2},
    {"F3", SDL_SCANCODE_F3},
    {"F4", SDL_SCANCODE_F4},
    {"F5", SDL_SCANCODE_F5},
    {"F6", SDL_SCANCODE_F6},
    {"F7", SDL_SCANCODE_F7},
    {"F8", SDL_SCANCODE_F8},
    {"F9", SDL_SCANCODE_F9},
    {"F10", SDL_SCANCODE_F10},
    {"F11", SDL_SCANCODE_F11},
    {"F12", SDL_SCANCODE_F12},
    {"UP", SDL_SCANCODE_UP},
    {"DOWN", SDL_SCANCODE_DOWN},
    {"RIGHT", SDL_SCANCODE_RIGHT},
    {"LEFT", SDL_SCANCODE_LEFT},
    {"LALT", SDL_SCANCODE_LALT},
    {"LCTRL", SDL_SCANCODE_LCTRL},
    {"LSHIFT", SDL_SCANCODE_LSHIFT},
    {"RALT", SDL_SCANCODE_RALT},
    {"RCTRL", SDL_SCANCODE_RCTRL},
    {"RSHIFT", SDL_SCANCODE_RSHIFT},
    {"ESCAPE", SDL_SCANCODE_ESCAPE}
};

static int
lookup_bind(config_t *cfg_t, char *str, SDL_Scancode *n) {
    char *tmp;

    config_lookup_string(cfg_t, str, (const char **)&tmp);
    
    for (size_t i = 0; i < nelem(Keylist); ++i)
	if (! strcmp(tmp, Keylist[i].t)) {
	    *n = Keylist[i].s;

	    return 0;
	}

    return 1;
}

int
load_bind(qik_bind *bind, config_t *cfg_t) {
    if (lookup_bind(cfg_t, "bind.dlook", &bind->dlook))
	return 1;
    
    if (lookup_bind(cfg_t, "bind.rlook", &bind->rlook))
	return 1;
    
    if (lookup_bind(cfg_t, "bind.llook", &bind->llook))
	return 1;
    
    if (lookup_bind(cfg_t, "bind.fmove", &bind->fmove))
	return 1;
    
    if (lookup_bind(cfg_t, "bind.bmove", &bind->bmove))
	return 1;
    
    if (lookup_bind(cfg_t, "bind.rmove", &bind->rmove))
	return 1;
    
    if (lookup_bind(cfg_t, "bind.lmove", &bind->lmove))
	return 1;
    
    if (lookup_bind(cfg_t, "bind.pfire", &bind->pfire))
	return 1;
    
    if (lookup_bind(cfg_t, "bind.sfire", &bind->sfire))
	return 1;
    
    if (lookup_bind(cfg_t, "bind.tfire", &bind->tfire))
	return 1;
    
    return 0;
}

/*
W     = Move Forward*
S     = Move Backward*
A     = Strafe Left*
D     = Strafe Right*

UP    = Look Up
DOWN  = Look Down
LEFT  = Rotate Left*
RIGHT = Rotate Right*

MOUSE = Mouselook
LCTRL = Pfire
LSHIFT = Run
LALT = Sfire
ESCAPE = Quit *
*/

void
get_bind(void)
{
    /* Vertical position of the camera */
    
    glb_usr.z_pos = glb_cfg.h / 2;
    
    glb_usr.mov_speed = glb_time_frame * (glb_map.speed * 2);

    glb_usr.rot_speed = glb_time_frame * glb_map.speed;

    /* Pump the event loop, gathering states from input */
    
    SDL_PumpEvents();
    
    Uint8 *Keystate = SDL_GetKeyboardState(NULL);

    if (Keystate[glb_bind.fmove]) {
	if (!glb_map.world.tile[(unsigned)(glb_usr.x_pos + glb_usr.x_dir * glb_usr.mov_speed)][(unsigned)glb_usr.y_pos].w)
	    glb_usr.x_pos += glb_usr.x_dir * glb_usr.mov_speed;
	if (!glb_map.world.tile[(unsigned)glb_usr.x_pos][(unsigned)(glb_usr.y_pos + glb_usr.y_dir * glb_usr.mov_speed)].w)
	    glb_usr.y_pos += glb_usr.y_dir * glb_usr.mov_speed;
    }

    if (Keystate[glb_bind.bmove]) {
	if (!glb_map.world.tile[(unsigned)(glb_usr.x_pos - glb_usr.x_dir * glb_usr.mov_speed)][(unsigned)glb_usr.y_pos].w)
	    glb_usr.x_pos -= glb_usr.x_dir * glb_usr.mov_speed;
	if (!glb_map.world.tile[(unsigned)glb_usr.x_pos][(unsigned)(glb_usr.y_pos - glb_usr.y_dir * glb_usr.mov_speed)].w)
	    glb_usr.y_pos -= glb_usr.y_dir * glb_usr.mov_speed;
    }

    if (Keystate[glb_bind.rmove]) {
	if (!glb_map.world.tile[(unsigned)(glb_usr.x_pos + glb_usr.x_plane * glb_usr.mov_speed)][(unsigned)glb_usr.y_pos].w)
	    glb_usr.x_pos += glb_usr.x_plane * glb_usr.mov_speed;
	if (!glb_map.world.tile[(unsigned)glb_usr.x_pos][(unsigned)(glb_usr.y_pos + glb_usr.y_plane * glb_usr.mov_speed)].w)
	    glb_usr.y_pos += glb_usr.y_plane * glb_usr.mov_speed;
    }

    if (Keystate[glb_bind.lmove]) {
	if (!glb_map.world.tile[(unsigned)(glb_usr.x_pos - glb_usr.x_plane * glb_usr.mov_speed)][(unsigned)glb_usr.y_pos].w)
	    glb_usr.x_pos -= glb_usr.x_plane * glb_usr.mov_speed;
	if (!glb_map.world.tile[(unsigned)glb_usr.x_pos][(unsigned)(glb_usr.y_pos - glb_usr.y_plane * glb_usr.mov_speed)].w)
	    glb_usr.y_pos -= glb_usr.y_plane * glb_usr.mov_speed;
    }

    if (Keystate[glb_bind.ulook]) {
    }

    if (Keystate[glb_bind.dlook]) {
    }

    if (Keystate[glb_bind.rlook]) {
	glb_usr.x_dir_old = glb_usr.x_dir;
	glb_usr.x_dir = glb_usr.x_dir * cos(-glb_usr.rot_speed) - glb_usr.y_dir * sin(-glb_usr.rot_speed);
	glb_usr.y_dir = glb_usr.x_dir_old * sin(-glb_usr.rot_speed) + glb_usr.y_dir * cos(-glb_usr.rot_speed);
	glb_usr.x_plane_old = glb_usr.x_plane;
	glb_usr.x_plane = glb_usr.x_plane * cos(-glb_usr.rot_speed) - glb_usr.y_plane * sin(-glb_usr.rot_speed);
	glb_usr.y_plane = glb_usr.x_plane_old * sin(-glb_usr.rot_speed) + glb_usr.y_plane * cos(-glb_usr.rot_speed);
    }

    if (Keystate[glb_bind.llook]) {
	glb_usr.x_dir_old = glb_usr.x_dir;
	glb_usr.x_dir = glb_usr.x_dir * cos(glb_usr.rot_speed) - glb_usr.y_dir * sin(glb_usr.rot_speed);
	glb_usr.y_dir = glb_usr.x_dir_old * sin(glb_usr.rot_speed) + glb_usr.y_dir * cos(glb_usr.rot_speed);
	glb_usr.x_plane_old = glb_usr.x_plane;
	glb_usr.x_plane = glb_usr.x_plane * cos(glb_usr.rot_speed) - glb_usr.y_plane * sin(glb_usr.rot_speed);
	glb_usr.y_plane = glb_usr.x_plane_old * sin(glb_usr.rot_speed) + glb_usr.y_plane * cos(glb_usr.rot_speed);
    }
    
    if (Keystate[glb_bind.pfire]) {
	exit(0);
    }

    if (Keystate[glb_bind.sfire]) {
    }
	
    if (Keystate[glb_bind.tfire]) {
    }

    return;
}
