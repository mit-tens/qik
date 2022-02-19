#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <libconfig.h>
#include "defs.h"
#include "types.h"
#include "glb.h"
#include "common.h"
#include "error.h"
#include "cfg_t.h"
#include "bind.h"
#include "map.h"
#include "rend.h"

/* It's wiser to initialize these to real values instead of NULL or zero, in that they're still valid unmodified. */

static char *initcfg = QIK_INITCFG, *initmap = QIK_INITMAP, *initusr = QIK_INITUSR;

static unsigned initw = QIK_INITW, inith = QIK_INITH;

static bool use_initcfg = false, use_initmap = false, use_initusr = false, use_initscr = false;

static int
load_cfg(void)
{
    if (get_cfg_t(&glb_cfg_cfg_t, use_initcfg ? initcfg : QIK_INITCFG))
	return 1;

    if (use_initscr)
	glb_cfg.w = initw, glb_cfg.h = inith;
    else
	if ((! config_lookup_int(&glb_cfg_cfg_t, "w", (int *)&glb_cfg.w)) ||
	    (! config_lookup_int(&glb_cfg_cfg_t, "h", (int *)&glb_cfg.h)))
	    
	    glb_cfg.w = QIK_INITW, glb_cfg.h = QIK_INITH;
    
    if (use_initusr)
	glb_cfg.usr = initusr;
    else
	if (! config_lookup_string(&glb_cfg_cfg_t, "usr", (const char **)&glb_cfg.usr))
	    glb_cfg.usr = QIK_INITUSR;

    glb_usr.name = glb_cfg.usr;

    if (use_initmap)
	glb_cfg.map = initmap;
    else
	if (! config_lookup_string(&glb_cfg_cfg_t, "map", (const char **)&glb_cfg.map))
	    glb_cfg.map = QIK_INITMAP;
    
    return 0;
}

static int
init_sdl(void)
{
    if (SDL_Init(SDL_INIT_VIDEO))
	return 1;

    if ((glb_window = SDL_CreateWindow(QIK_VERSION, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, glb_cfg.w, glb_cfg.h, 0)) == NULL)
	return 1;
    
    if ((glb_format = SDL_GetWindowPixelFormat(glb_window)) == SDL_PIXELFORMAT_UNKNOWN)
	return 1;

    if ((glb_renderer = SDL_CreateRenderer(glb_window, -1, SDL_RENDERER_ACCELERATED)) == NULL)
	return 1;

    return 0;
}

static int
main_args(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
	if ((strcmp(argv[i], "-cfg") == 0)) {
	    initcfg = argv[++i];

	    use_initcfg = true;
	}

	else if ((strcmp(argv[i], "-map") == 0)) {
	    initmap = argv[++i];

	    use_initmap = true;
	}

	else if ((strcmp(argv[i], "-scr") == 0)) {
	    initw = atoi(argv[++i]);

	    inith = atoi(argv[++i]);

	    use_initscr = true;
	}

	else if ((strcmp(argv[i], "-usr") == 0)) {
	    initusr = argv[++i];

	    use_initusr = true;
	}
	    
	else
	    return 1;
    }

    return 0;
}

int
main(int argc, char *argv[])
{
    fprintf(stderr, "%s\nCopyright %s %s\n", QIK_VERSION, QIK_CPYYEAR, QIK_AUTHORS);
    
    if (main_args(argc, argv))
	goto usage;

    if (load_cfg())
	goto scram;

    if (load_bind(&glb_bind, &glb_cfg_cfg_t))
	goto scram;

    if (init_sdl())
	goto scram;

    if (load_map(&glb_map, &glb_usr, &glb_mtex, &glb_mspr, &glb_map_cfg_t, glb_cfg.map, glb_format))
	goto scram;

    srand(time(NULL));
                
    glb_time = 0;

    glb_time_old = 0;

    glb_running = true;

    while (glb_running) {
        get_rend();
        get_bind();
    }

    return EXIT_SUCCESS;
    
scram:
    error_iprint(); /* Finest error handling offered */

    return EXIT_FAILURE;

usage:
    fprintf(stderr, "usage: qik [options ..]\n");
    fprintf(stderr, "-cfg       <file>    Use config file\n");
    fprintf(stderr, "-map       <file>    Load to map file\n");
    fprintf(stderr, "-scr       <w> <h>   Override screen size\n");
    fprintf(stderr, "-usr       <string>  Username for player\n"); 
    fprintf(stderr, "-fhqwhgads           You are here\n");

    return EXIT_FAILURE;
}
