#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <libconfig.h>
#include "common.h"
#include "types.h"
#include "cfg_t.h"
#include "load.h"
#include "map.h"

static qik_mtex
alloc_mtex(const config_t *cfg_t, const Uint32 format)
{
    config_setting_t *set_tex;

    if ((set_tex = config_lookup(cfg_t, "tex")) == NULL)
	return (qik_mtex){ 0 };

    unsigned ntex = config_setting_length(set_tex);

    qik_mtex mtex = (qik_mtex){.n=ntex, .ref=malloc(ntex * sizeof(SDL_Surface *))};

    for (unsigned i = 0; i < ntex; ++i)
	mtex.ref[i] = load_surface(config_setting_get_string_elem(set_tex, i), format);

    return mtex;
}

static qik_mspr
alloc_mspr(const config_t *cfg_t)
{
    config_setting_t *set_spr;

    if ((set_spr = config_lookup(cfg_t, "spr")) == NULL)
	return (qik_mspr){ 0 };

    unsigned nspr = config_setting_length(set_spr);
    
    qik_mspr mspr = (qik_mspr){.n=nspr, .ref=malloc(nspr * sizeof(qik_spr))};

    for (unsigned i = 0; i < nspr; ++i)
	mspr.ref[i] = (qik_spr) {
	    config_setting_get_float_elem(config_setting_get_elem(set_spr, i), 0),
	    config_setting_get_float_elem(config_setting_get_elem(set_spr, i), 1),
	    config_setting_get_int_elem(config_setting_get_elem(set_spr, i), 2)
	};

    return mspr;
}

static qik_map_cell **
alloc_world_ref(const unsigned cols, const unsigned rows)
{
    qik_map_cell **ref = malloc(rows * sizeof(qik_map_cell *));

    for (unsigned i = 0; i < rows; ++i)
	ref[i] = malloc(cols * sizeof(qik_map_cell));

    return ref;
}

static void
free_mtex(qik_mtex *mtex)
{
    for (unsigned i = 0; i < mtex->n; ++i)
	free(mtex->ref[i]);

    free(mtex->ref);

    *mtex = (qik_mtex){ 0 };

    return;
}

static void
free_mspr(qik_mspr *mspr)
{
    free(mspr->ref);

    *mspr = (qik_mspr){ 0 };

    return;
}

static void
free_world_ref(qik_map_cell **ref, const unsigned rows)
{
    for (unsigned i = 0; i < rows; ++i)
	free(ref[i]);
    
    free(ref);

    return;
}

static void
free_map(qik_map *map)
{
    free_world_ref(map->world.tile, map->world.h);
    free_world_ref(map->world.roof, map->world.h);
    
    *map = (qik_map){ 0 };

    return;
}

static void
free_usr(qik_usr *usr)
{
    *usr = (qik_usr){ 0 };

    return;
}

static int
set_bool(qik_map *map, const config_t *cfg_t)
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

    return 0;
}

static int
set_world(qik_map *map, const config_t *cfg_t)
{
    config_setting_t *set_tile, *set_roof;

    if ((set_tile = config_lookup(cfg_t, "world.tile")) == NULL)
	return 1;

    if ((set_roof = config_lookup(cfg_t, "world.roof")) == NULL)
	return 1;

    unsigned w_tile = (unsigned)config_setting_length(config_setting_get_elem(set_tile, 0));
    unsigned h_tile = (unsigned)config_setting_length(set_tile);
    unsigned w_roof = (unsigned)config_setting_length(config_setting_get_elem(set_roof, 0));
    unsigned h_roof = (unsigned)config_setting_length(set_roof);

    if (w_tile != w_roof || h_tile != h_roof)
	return 1;

    map->world.w = w_tile;
    map->world.h = h_tile;

    if ((map->world.tile = alloc_world_ref(map->world.w, map->world.h)) == NULL)
	return 1;

    if ((map->world.roof = alloc_world_ref(map->world.w, map->world.h)) == NULL)
	return 1;
    
    for (unsigned x = 0; x < map->world.w; ++x)
	for (unsigned y = 0; y < map->world.h; ++y) {
	    map->world.tile[x][y] = (qik_map_cell) {
		config_setting_get_float_elem(config_setting_get_elem(config_setting_get_elem(set_tile, y), x), 0),
		config_setting_get_int_elem(config_setting_get_elem(config_setting_get_elem(set_tile, y), x), 1),
		config_setting_get_int_elem(config_setting_get_elem(config_setting_get_elem(set_tile, y), x), 2),
	    };
	    
	    map->world.roof[x][y] = (qik_map_cell) {
		config_setting_get_float_elem(config_setting_get_elem(config_setting_get_elem(set_roof, y), x), 0),
		config_setting_get_int_elem(config_setting_get_elem(config_setting_get_elem(set_roof, y), x), 1),
		config_setting_get_int_elem(config_setting_get_elem(config_setting_get_elem(set_roof, y), x), 2),
	    };
	}

    config_lookup_float(cfg_t, "world.master", &map->world.master);

    return 0;
}

static int
set_usr(qik_usr *usr, const config_t *cfg_t)
{
    config_lookup_float(cfg_t, "usr.x_pos", &usr->x_pos);
    config_lookup_float(cfg_t, "usr.y_pos", &usr->y_pos);
    config_lookup_float(cfg_t, "usr.x_dir", &usr->x_dir);
    config_lookup_float(cfg_t, "usr.y_dir", &usr->y_dir);
    config_lookup_float(cfg_t, "usr.x_plane", &usr->x_plane);
    config_lookup_float(cfg_t, "usr.y_plane", &usr->y_plane);

    return 0;
}

static int
set_string(qik_map *map, const config_t *cfg_t)
{
    config_lookup_string(cfg_t, "title", (const char **)&map->title);
    config_lookup_string(cfg_t, "version", (const char **)&map->version);
    config_lookup_string(cfg_t, "music", (const char **)&map->music);

    return 0;
}

static int
set_int(qik_map *map, const config_t *cfg_t)
{
    config_lookup_int(cfg_t, "speed", &map->speed);

    if (map->cfg.fog) {
	config_lookup_int(cfg_t, "fog.r", (int *)&map->fog.c.r);
	config_lookup_int(cfg_t, "fog.g", (int *)&map->fog.c.g);
	config_lookup_int(cfg_t, "fog.b", (int *)&map->fog.c.b);
	config_lookup_int(cfg_t, "fog.f", &map->fog.f);
    }

    return 0;
}

static int
set_mtex(qik_mtex *mtex, const config_t *cfg_t, const Uint32 format)
{
    *mtex = alloc_mtex(cfg_t, format);

    if (mtex == NULL)
	return 1;

    return 0;
}

static int
set_mspr(qik_mspr *mspr, const config_t *cfg_t)
{
    *mspr = alloc_mspr(cfg_t);

    if (mspr == NULL)
	return 1;

    return 0;
}

int
load_map(qik_map *map, qik_usr *usr, qik_mtex *mtex, qik_mspr *mspr, config_t *cfg_t, const char *path, const Uint32 format)
{
    if (get_cfg_t(cfg_t, path))
	return 1;

    if (set_bool(map, cfg_t))
	return 1;

    if (set_world(map, cfg_t))
	return 1;

    if (set_usr(usr, cfg_t))
	return 1;

    if (set_string(map, cfg_t))
	return 1;

    if (set_int(map, cfg_t))
	return 1;

    if (set_mtex(mtex, cfg_t, format))
	return 1;

    if (set_mspr(mspr, cfg_t))
	return 1;

    return 0;
}

void
unload_map(qik_map *map, qik_usr *usr, qik_mtex *mtex, qik_mspr *mspr, config_t *cfg_t)
{
    free_mtex(mtex);

    free_mspr(mspr);

    free_map(map);

    free_usr(usr);

    config_destroy(cfg_t);

    return;
}
