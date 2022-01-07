#include <stdbool.h>
#include <SDL2/SDL.h>
#include <libconfig.h>
#include "types.h"
#include "cfg_t.h"
#include "cfg.h"

#define USE_CFG_INITMAP 1

int
load_cfg(qik_cfg *cfg, qik_usr *usr, config_t *cfg_t, const char *path)
{
    if (get_cfg_t(cfg_t, path))
	return 1;

    if (! config_lookup_int(cfg_t, "w", (int *)&cfg->w))
	return 1;

    if (! config_lookup_int(cfg_t, "h", (int *)&cfg->h))
	return 1;

    if (! config_lookup_string(cfg_t, "usr", (const char **)&cfg->usr))
	return 1;
    
#ifdef USE_CFG_INITMAP
    if (! config_lookup_string(cfg_t, "map", (const char **)&cfg->map))
	return 1;
#endif
    
    usr->name = cfg->usr;
    
    return 0;
}
