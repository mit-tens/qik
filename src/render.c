#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <libconfig.h>
#include "qik/types.h"
#include "global.h"
#include "render.h"

#define  tid(x, y) global_map.world.tile[x][y]
#define  rid(x, y) global_map.world.roof[x][y]
#define wtid(x, y) tid(x, y).w
#define wrid(x, y) rid(x, y).w
#define ftid(x, y) tid(x, y).f
#define frid(x, y) rid(x, y).f
#define htid(x, y) tid(x, y).h
#define hrid(x, y) rid(x, y).h

#define ixpos (int)global_usr.x_pos
#define iypos (int)global_usr.y_pos

static inline SDL_Surface *
getmSurface(int id)
{
    return (id >= sizeof mSurface) ?
	mSurface[0] :
	mSurface[id] ;
}

static double *zBuffer = NULL;
static double *dSprite = NULL;

static inline void
alloc_zBuffer(void)
{
    zBuffer = malloc(gConfig.w * sizeof(double));
}

static inline void
alloc_rend_spr_dist(void)
{
    dSprite = malloc(gConfig.n * sizeof(double));
}

static void
rend_floor(void)
{
    /* rayDir for leftmost ray (x = 0) and rightmost ray (x = global_cfg.w) */

    double rayDirX0 = global_usr.x_dir - global_usr.x_plane;

    double rayDirY0 = global_usr.y_dir - global_usr.y_plane;

    double rayDirX1 = global_usr.x_dir + global_usr.x_plane;

    double rayDirY1 = global_usr.y_dir + global_usr.y_plane;
    
    for (int y = 0; y < global_cfg.h; ++y) {		
	/* Horizontal distance from the camera to the floor for the current row. */
	/* 0.5 is the z position exactly in the middle between floor and ceiling. */

	double rowDistance = global_usr.z_pos / (y - global_usr.z_pos);
	
	/* Calculate the real world step vector we have to add for each x (parallel to camera plane) */
	/* Adding step by step avoids multiplications with a weight in the inner loop */

	double floorStepX = rowDistance * (rayDirX1 - rayDirX0) / global_cfg.w;

	double floorStepY = rowDistance * (rayDirY1 - rayDirY0) / global_cfg.w;
		
	/* Real world coordinates of the leftmost column. This will be updated as we step to the right. */

	double floorX = global_usr.x_pos + rowDistance * rayDirX0;

	double floorY = global_usr.y_pos + rowDistance * rayDirY0;
		
	for (int x = 0; x < global_cfg.w; ++x) {
	    int cellX = (int)floorX;

	    int cellY = (int)floorY;

	    SDL_Surface *real = getmtex(ftid(cellX, cellY));
	    
	    /* get the texture coordinate from the fractional part */

	    int texX = (int)(real->w * (floorX - cellX)) & (real->w - 1);

	    int texY = (int)(real->h * (floorY - cellY)) & (real->h - 1);
	    
	    floorX += floorStepX;

	    floorY += floorStepY;
            
	    SDL_Color rgb = getpixel(real, texX, texY);

	    drawpixel(global_renderer, rgb, x, y);

	    if (global_map.cfg.ceil) {
		real = getmtex(frid(cellX, cellY));
		
		rgb = getpixel(real, texX, texY);

		/* symmetrical, at screenHeight - y - 1 instead of y */
		    
		drawpixel(global_renderer, rgb, x, global_cfg.h - y - 1);
	    }
	}
    }

    return;
}

static void
rend_walls(void)
{
    for (int x = 0; x < global_cfg.w; ++x) {
	double cameraX = 2 * x / (float)global_cfg.w - 1;

	double rayDirX = global_usr.x_dir + global_usr.x_plane * cameraX;

	double rayDirY = global_usr.y_dir + global_usr.y_plane * cameraX;
	
	/* Length of ray from one X or Y side to next X or Y side */

	double deltaDistX = fabs(1 / rayDirX);

	double deltaDistY = fabs(1 / rayDirY);

	/* What direction to step in X or Y direction (either +1 or -1) */

	int stepX = (rayDirX < 0) ?
	    -1 :
	    1 ;

	int stepY = (rayDirY < 0) ?
	    -1 :
	    1 ;

	/* Length of ray from current position to next X or Y side */
	
	double sideDistX = (rayDirX < 0) ?
	    (global_usr.x_pos - ixpos) * deltaDistX :
	    (ixpos + 1.00 - global_usr.x_pos) * deltaDistX ;

	double sideDistY = (rayDirY < 0) ?
	    (global_usr.y_pos - iypos) * deltaDistY :
	    (iypos + 1.00 - global_usr.y_pos) * deltaDistY ;

	int mapX = ixpos;
	    
	int mapY = iypos;

	/* Jump to next grid square, OR in X direction, OR in Y direction */

	/* tswitch flags which of the two map sections were hit */

	/* tile = 0; roof = 1; */

	bool tswitch = 0;

	bool side = 0;
	
	for (;;) {
	    if (sideDistX < sideDistY) {
		sideDistX += deltaDistX;

		mapX += stepX;

		side = 0;
	    }
	    else {
		sideDistY += deltaDistY;

		mapY += stepY;

		side = 1;
	    }

	    if (wtid(mapX, mapY))
		tswitch = 0, break;

	    if (wrid(mapX, mapY))
		tswitch = 1, break;
	}
	    
	/* Calculate distance projected on camera direction */

	double perpWallDist = side ?
	    sideDistY - deltaDistY :
	    sideDistX - deltaDistX ;

	/* Calculate lowest and highest pixel to fill in current stripe */
	
	int lineHeight = global_cfg.h / perpWallDist;

	int drawStart = (-lineHeight + global_cfg.h) / 2;

	if (drawStart > global_cfg.h) drawStart = 0;
	
	int drawEnd = (lineHeight + global_cfg.h) / 2;

	if (drawEnd > global_cfg.h) drawEnd = global_cfg.h;
		
	double wallX = side ?
	    global_usr.x_pos + perpWallDist * rayDirX :
	    global_usr.y_pos + perpWallDist * rayDirY ;

	wallX -= floor(wallX);

	SDL_Surface *real = getmtex(tswitch ? wrid(mapX, mapY) : wtid(mapX, mapY));

	int texX = (int)(wallX * (double)real->w);

	if ((side == 0 && rayDirX > 0) ||
	    (side == 1 && rayDirY < 0))
	    
	    texX = real->w - texX - 1;

	double step = 1.00 * real->h / lineHeight;

	double tex_pos = (drawStart - global_cfg.h / 2 + lineHeight / 2) * step;

	for (int y = drawStart; y < drawEnd; ++y) {
	    int texY = (int)tex_pos & (real->h - 1);

	    tex_pos += step;

	    SDL_Color rgb = global_map.cfg.fog ?
		interpolate_rgb(getpixel(real, texX, texY), global_map.fog.c, perpWallDist / global_map.fog.f) :
		getpixel(real, texX, texY) ;
	    
	    if (global_map.cfg.shadows && side)
		{rgb.r /= 2; rgb.g /= 2; rgb.b /= 2;}

	    drawpixel(global_renderer, rgb, x, y);
	}
	
	rend_z_buffer[x] = perpWallDist;
    }

    return;
}

static void
rend_sprites(void)
{
    int spriteOrder[global_mspr.n];
    
    /* Sort sprites from far to close */

    for (unsigned i = 0; i < global_mspr.n; ++i) {
        spriteOrder[i] = i;

	rend_spr_dist[i] = ((global_usr.x_pos - global_mspr.ref[i].x) *
			    (global_usr.x_pos - global_mspr.ref[i].x) +
			    (global_usr.y_pos - global_mspr.ref[i].y) *
			    (global_usr.y_pos - global_mspr.ref[i].y));
    }
    
    for (unsigned i = 0; i < global_mspr.n; ++i) {
	/* Translate sprite position relative to camera */

	double spriteX = global_mspr.ref[spriteOrder[i]].x - global_usr.x_pos;

	double spriteY = global_mspr.ref[spriteOrder[i]].y - global_usr.y_pos;

	/* Transform sprite with inverse camera matrix */

	double invDet = 1.00 / (global_usr.x_plane * global_usr.y_dir - global_usr.x_dir * global_usr.y_plane); /* Required for correct matrix multiplicaton */

	double transformX = invDet * (global_usr.y_dir * spriteX - global_usr.x_dir * spriteY);

	double transformY = invDet * (-global_usr.y_plane * spriteX + global_usr.x_plane * spriteY); /* The depth inside the screen, Z in 3D */
        
	int spriteScreenX = (int)((global_cfg.w / 2) * (1 + transformX / transformY));
        
	/* Calcluate height of sprite on screen */

	int spriteHeight = abs((int)(global_cfg.h / transformY)); /* Using transformY instead of real distance negates fisheye */

	/* Calculate lowest and highest pixel to fill in stripe */

	int drawStartY = (-spriteHeight + global_cfg.h) / 2;

	if (drawStartY < 0) drawStartY = 0;

	int drawEndY = (spriteHeight + global_cfg.h) / 2;

	if (drawEndY >= (int)global_cfg.h) drawEndY = global_cfg.h;
        
	/* Calculate width of sprite TODO: Don't just use the width */

	int spriteWidth = spriteHeight;

	int drawStartX = -spriteWidth / 2 + spriteScreenX;

	if (drawStartX < 0) drawStartX = 0;

	int drawEndX = spriteWidth / 2 + spriteScreenX;

	if (drawEndX >= (int)global_cfg.w) drawEndX = global_cfg.w;

	SDL_Surface *real = getmtex(global_mspr.ref[spriteOrder[i]].tex);
        
	for (int str = drawStartX; str < drawEndX; ++str) {
	    int texX = (int)(256 * (str - (-spriteWidth / 2 + spriteScreenX)) * real->w / spriteWidth) / 256;

	    /* Conditions:
	       1) it's in front of the camera plane
	       2) it's on the screen (left)
	       3) it's on the screen (right)
	       4) rend_z_buffer, with perpendicular distance */

	    if (transformY > 0
		&& str > 0
		&& str < (int)global_cfg.w
		&& transformY < rend_z_buffer[str])
		for (int y = drawStartY; y < drawEndY; ++y) {
		    int d = y * 256 - global_cfg.h * 128 + spriteHeight * 128; //256 and 128 factors to avoid doubles

		    int texY = d * real->h / spriteHeight / 256;

		    SDL_Color rgb = getpixel(real, texX, texY);
                
		    if (rgb.r != 0x00 && rgb.g != 0x00 && rgb.b != 0x00)
			drawpixel(global_renderer, rgb, str, y);
		}
	}
    }

    return;
}

void
render(void)
{
    if (rend_z_buffer == NULL)
	alloc_rend_z_buffer();

    if (rend_spr_dist == NULL)
	alloc_rend_spr_dist();
    
    SDL_RenderClear(global_renderer);
    
    if (global_map.cfg.floor)
	rend_floor();

    if (global_map.cfg.walls)
	rend_walls();

    if (global_map.cfg.sprites)
	rend_sprites();
    
    SDL_RenderPresent(global_renderer);
    
    return;
}
