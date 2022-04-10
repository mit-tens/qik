#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <libconfig.h>
#include "qik/defs.h"
#include "qik/types.h"
#include "global.h"
#include "qik/common.h"
#include "qik/error.h"
#include "config.h"
#include "control.h"
#include "map.h"
#include "render.h"

/* It's wiser to initialize these to real values instead of NULL or zero, in that they're still valid unmodified. */

static char *initcfg = QIK_INITCFG, *initmap = QIK_INITMAP, *initusr = QIK_INITUSR;

static int initw = QIK_INITW, inith = QIK_INITH;

static bool use_initcfg = false, use_initmap = false, use_initusr = false, use_initscr = false;

static int
cfg(void)
{
	if (retrieve_config(&gconfig_t, use_initcfg ? initcfg : QIK_INITCFG))
		return 1;

	if (use_initscr)
		gConfig.w = initw, gConfig.h = inith;
	else
	if ((! config_lookup_int(&gconfig_t, "w", (int *)&gConfig.w)) ||
	    (! config_lookup_int(&gconfig_t, "h", (int *)&gConfig.h)))
	    gConfig.w = QIK_INITW, gConfig.h = QIK_INITH;

	if (use_initusr)
		gConfig.usr = initusr;
	else
	if (! config_lookup_string(&gconfig_t, "usr", (const char **)&gConfig.usr))
	    gConfig.usr = QIK_INITUSR;

	gUser.name = gConfig.usr;

	if (use_initmap)
		gConfig.map = initmap;
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

	if ((gWindow = SDL_CreateWindow(QIK_VERSION,
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					glb_cfg.w,
					glb_cfg.h,
					0
	)) == NULL)
		return 1;

	if ((gFormat = SDL_GetWindowPixelFormat(gWindow)) == SDL_PIXELFORMAT_UNKNOWN)
		return 1;

	if ((gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED)) == NULL)
		return 1;

	return 0;
}

static int
args(int argc, char *argv[])
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
    
    if (args(argc, argv))
	goto usage;

    if (cfg())
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
	get_frames();
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
    fprintf(stderr, "-usr       <name>    Username for player\n"); 
    fprintf(stderr, "-fhqwhgads           You are here\n");

    return EXIT_FAILURE;
}
