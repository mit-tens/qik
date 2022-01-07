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
#include "cfg.h"
#include "bind.h"
#include "map.h"
#include "rend.h"

int
init_sdl(void)
{
    if (SDL_Init(SDL_INIT_VIDEO))
	return 1;
    
    if ((glb_window = SDL_CreateWindow("Qik", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, glb_cfg.w, glb_cfg.h, 0)) == NULL)
	return 1;
                              
    if ((glb_format = SDL_GetWindowPixelFormat(glb_window)) == SDL_PIXELFORMAT_UNKNOWN)
	return 1;

    if ((glb_renderer = SDL_CreateRenderer(glb_window, -1, SDL_RENDERER_ACCELERATED)) == NULL)
	return 1;

    return 0;
}

static char *initcfg = NULL;
static char *initmap = NULL;
static unsigned initw = 0;
static unsigned inith = 0;

int
main_args(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
	if ((strcmp(argv[i], "-c") == 0) ||
	    (strcmp(argv[i], "--config") == 0))
	    initcfg = argv[++i];

	else if ((strcmp(argv[i], "-m") == 0) ||
		 (strcmp(argv[i], "--map") == 0))
	    initmap = argv[++i];

	else if ((strcmp(argv[i], "-s") == 0) ||
		 (strcmp(argv[i], "--screen") == 0))
	    inith = (unsigned)atol(argv[++i]);
	    
	else
	    return 1;
    }

    if (initcfg == NULL)
	initcfg = QIK_INITCFG;

    if (initmap == NULL)
	initmap = QIK_INITMAP;

    return 0;
}

int
main(int argc, char *argv[])
{
    fprintf(stderr, "Qik %s\nCopyright %s %s\n", QIK_VERSION, QIK_CPYYEAR, QIK_AUTHORS);
    
    if (main_args(argc, argv))
	goto usage;

    if (load_cfg(&glb_cfg, &glb_usr, &glb_cfg_cfg_t, initcfg))
	goto scram;

    if (load_bind(&glb_bind, &glb_cfg_cfg_t))
	goto scram;

    if (init_sdl())
	goto scram;

    if (load_map(&glb_map, &glb_usr, &glb_mtex, &glb_mspr, &glb_map_cfg_t, initmap, glb_format))
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
    fprintf(stderr, "-c, --config       <file>    Use config file\n");
    fprintf(stderr, "-m, --map          <file>    Load to map file\n");
    fprintf(stderr, "-s, --screen       <w> <h>   Override screen size\n");
    fprintf(stderr, "-f, --fhqwhgads              You are here\n");

    return EXIT_FAILURE;
}
