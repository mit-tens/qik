#ifndef MAP_H
#define MAP_H

int
load_map(qik_map *, qik_usr *, qik_mtex *, qik_mspr *, config_t *, char *, Uint32);

void
unload_map(qik_map *, qik_usr *, qik_mtex *, qik_mspr *, config_t *);

#endif
