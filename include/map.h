#ifndef MAP_H
#define MAP_H

int
load_map(qik_map *map, qik_usr *usr, qik_mtex *mtex, qik_mspr *mspr, config_t *cfg_t, const char *path, const Uint32 format);

void
unload_map(qik_map *map, qik_usr *usr, qik_mtex *mtex, qik_mspr *mspr, config_t *cfg_t);

#endif
