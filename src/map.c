#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <libconfig.h>
#include "qik/common.h"
#include "qik/types.h"
#include "config.h"
#include "gfx/image.h"
#include "map.h"

static SDL_Surface **
alloc_gSurface(SDL_Surface **gSurface, config_t *cfg_t, Uint32 format)
{
	config_setting_t *set_tex;

	if ((set_tex = config_lookup(cfg_t, "tex")) == NULL)
		return NULL;

	int ntex = config_setting_length(set_tex);

	gSurface = malloc(ntex * sizeof(SDL_Surface *));

	for (int i = 0; i < ntex; ++i)
		gSurface[i] = load_surface(config_setting_get_string_elem(set_tex, i), format);

	return gSurface;
}

static Sprite *
alloc_gSprite(Sprite *gSprite, config_t *cfg_t)
{
	config_setting_t *set_spr;

	if ((set_spr = config_lookup(cfg_t, "spr")) == NULL)
		return NULL;

	int nspr = config_setting_length(set_spr);
    
	gSprite = malloc(nspr * sizeof(Sprite));

	for (int i = 0; i < nspr; ++i)
		gSprite[i] = (Sprite) {
			config_setting_get_float_elem(config_setting_get_elem(set_spr, i), 0),
			config_setting_get_float_elem(config_setting_get_elem(set_spr, i), 1),
			config_setting_get_int_elem(config_setting_get_elem(set_spr, i), 2)
		};

	return gSprite;
}

static Cell **
alloc_world_ref(Cell **ref, int cols, int rows)
{
	ref = malloc(rows * sizeof(Cell *));

	for (int i = 0; i < rows; ++i)
		ref[i] = malloc(cols * sizeof(Cell));

	return ref;
}

/* Used for freeing Map.world.* and gSurface */
static void
free_double_pointer(void **ptr, int size)
{
	for (int i = 0; i < size; ++i)
		free(ptr[i]);

	free(ptr);
}

static void
free_map(Map *map)
{
	free_double_pointer(map->world.tile, map->world.h);
	free_double_pointer(map->world.roof, map->world.h);
    
	*map = (Map){ 0 };
}

static Map *
set_bool(Map *map, config_t *cfg_t)
{
	config_lookup_bool(cfg_t, "cfg.walls",   (int *)&map->cfg.walls);
	config_lookup_bool(cfg_t, "cfg.floor",   (int *)&map->cfg.floor);
	config_lookup_bool(cfg_t, "cfg.ceil",    (int *)&map->cfg.ceil);
	config_lookup_bool(cfg_t, "cfg.roof",    (int *)&map->cfg.roof);
	config_lookup_bool(cfg_t, "cfg.tile",    (int *)&map->cfg.tile);
	config_lookup_bool(cfg_t, "cfg.sprites", (int *)&map->cfg.sprites);
	config_lookup_bool(cfg_t, "cfg.sky",     (int *)&map->cfg.sky);
	config_lookup_bool(cfg_t, "cfg.shadows", (int *)&map->cfg.shadows);
	config_lookup_bool(cfg_t, "cfg.fog",     (int *)&map->cfg.fog);

	return map;
}

static Map *
set_world(Map *map, config_t *cfg_t)
{
	config_setting_t *set_tile, *set_roof;

	if ((set_tile = config_lookup(cfg_t, "world.tile")) == NULL)
		return NULL;

	if ((set_roof = config_lookup(cfg_t, "world.roof")) == NULL)
		return NULL;

	int w_tile = config_setting_length(config_setting_get_elem(set_tile, 0));
	int h_tile = config_setting_length(set_tile);
	int w_roof = config_setting_length(config_setting_get_elem(set_roof, 0));
	int h_roof = config_setting_length(set_roof);

	if (w_tile != w_roof || h_tile != h_roof)
		return NULL;

	map->world.w = w_tile;
	map->world.h = h_tile;

	if (alloc_world_ref(map->world.tile, map->world.w, map->world.h) == NULL)
		return NULL;

	if (alloc_world_ref(map->world.roof, map->world.w, map->world.h) == NULL)
		return NULL;
    
	for (int x = 0; x < map->world.w; ++x)
		for (int y = 0; y < map->world.h; ++y) {
	    	map->world.tile[x][y] = (Cell) {
			config_setting_get_float_elem(config_setting_get_elem(config_setting_get_elem(set_tile, y), x), 0),
			config_setting_get_int_elem(config_setting_get_elem(config_setting_get_elem(set_tile, y), x), 1),
			config_setting_get_int_elem(config_setting_get_elem(config_setting_get_elem(set_tile, y), x), 2),
	 	   };
	    
		    map->world.roof[x][y] = (Cell) {
			config_setting_get_float_elem(config_setting_get_elem(config_setting_get_elem(set_roof, y), x), 0),
			config_setting_get_int_elem(config_setting_get_elem(config_setting_get_elem(set_roof, y), x), 1),
			config_setting_get_int_elem(config_setting_get_elem(config_setting_get_elem(set_roof, y), x), 2),
		    };
	}

	config_lookup_float(cfg_t, "world.master", &map->world.master);

	return map;
}

static Map *
set_string(Map *map, config_t *cfg_t)
{
	config_lookup_string(cfg_t, "title", (const char **)&map->title);
	config_lookup_string(cfg_t, "music", (const char **)&map->music);

	return map;
}

static Map *
set_int(Map *map, config_t *cfg_t)
{
	config_lookup_int(cfg_t, "speed", &map->speed);

	if (map->cfg.fog) {
		config_lookup_int(cfg_t, "fog.r", (int *)&map->fog.c.r);
		config_lookup_int(cfg_t, "fog.g", (int *)&map->fog.c.g);
		config_lookup_int(cfg_t, "fog.b", (int *)&map->fog.c.b);
		config_lookup_int(cfg_t, "fog.f", &map->fog.f);
	}

	return map;
}

static User *
set_user(User *user, config_t *cfg_t)
{
	config_lookup_float(cfg_t, "usr.x_pos", &user->x_pos);
	config_lookup_float(cfg_t, "usr.y_pos", &user->y_pos);
	config_lookup_float(cfg_t, "usr.x_dir", &user->x_dir);
	config_lookup_float(cfg_t, "usr.y_dir", &user->y_dir);
	config_lookup_float(cfg_t, "usr.x_plane", &user->x_plane);
	config_lookup_float(cfg_t, "usr.y_plane", &user->y_plane);

	return user;
}

int
load_map(Map *map, User *user, SDL_Surface **gSurface, Sprite *gSprite, config_t *cfg_t, char *path, Uint32 format)
{
	if (retrieve_config(cfg_t, path))
		return 1;

	if (set_bool(map, cfg_t) == NULL)
		return 1;

	if (set_map(map, cfg_t) == NULL)
		return 1

	if (set_string(map, cfg_t) == NULL)
		return 1;

	if (set_int(map, cfg_t) == NULL)
		return 1;

	if (set_user(user, cfg_t) == NULL)
		return 1;

	if (alloc_gSurface(gSurface, cfg_t, format) == NULL)
		return 1;

	if (alloc_gSprite(gSprite, cfg_t) == NULL)
		return 1;

	return 0;
}

void
unload_map(Map *map, User *user, SDL_Surface **gSurface, Sprite *gSprite, config_t *cfg_t)
{
	free_gSurface(gSurface);

	free(gSprite);

	free_map(map);

	free_user(user);

	config_destroy(cfg_t);

	return;
}
