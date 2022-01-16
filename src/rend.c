#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <libconfig.h>
#include "types.h"
#include "glb.h"
#include "rend.h"

#define  tid(x, y) glb_map.world.tile[x][y]
#define  rid(x, y) glb_map.world.roof[x][y]
#define wtid(x, y) tid(x, y).w 
#define wrid(x, y) rid(x, y).w
#define ftid(x, y) tid(x, y).f
#define frid(x, y) rid(x, y).f
#define htid(x, y) tid(x, y).h
#define hrid(x, y) rid(x, y).h

#define drawpixel(e, c, x, y) {\
    SDL_SetRenderDrawColor(e, c.r, c.g, c.b, SDL_ALPHA_OPAQUE);\
    SDL_RenderDrawPoint(e, x, y);\
}

#define uxpos (unsigned)glb_usr.x_pos
#define uypos (unsigned)glb_usr.y_pos

static Uint32
getrawpixel(const SDL_Surface *surface, const int x, const int y)
{
    int bpp = surface->format->BytesPerPixel;
    
    /* Here p is the address to the pixel we want to retrieve */

    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
	return *p;
	break;

    case 2:
	return *(Uint16 *)p;
	break;

    case 3:
	if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
	    return p[0] << 16 | p[1] << 8 | p[2];
	else
	    return p[0] | p[1] << 8 | p[2] << 16;
	break;

    case 4:
	return *(Uint32 *)p;
	break;

    default:
	return 0; /* shouldn't happen, but avoids warnings */
    }
}

static SDL_Color
getpixel(const SDL_Surface *real, const int x, const int y)
{
    SDL_Color rgb;

    SDL_GetRGB(getrawpixel(real, x, y), real->format, &rgb.r, &rgb.g, &rgb.b);

    return rgb;
}

static inline SDL_Color
interpolate_rgb(const SDL_Color rgb, const SDL_Color c, const float fpcnt) /* Here fpcnt should be perpWallDist / fog.f */
{
    return (SDL_Color) {
	(1 - (fpcnt > 1) ? 1 : fpcnt) * rgb.r + fpcnt * c.r,
	(1 - (fpcnt > 1) ? 1 : fpcnt) * rgb.g + fpcnt * c.g,
	(1 - (fpcnt > 1) ? 1 : fpcnt) * rgb.b + fpcnt * c.b,
	SDL_ALPHA_OPAQUE
    };
}

static inline SDL_Surface *
getmtex(const unsigned id)
{
    return (id >= glb_mtex.n) ?
	glb_mtex.ref[0] :
	glb_mtex.ref[id] ;
}

static float *rend_z_buffer;
static float *rend_spr_dist;

static void
rend_floor(void)
{
    /* rayDir for leftmost ray (x = 0) and rightmost ray (x = glb_cfg.w) */

    float rayDirX0 = glb_usr.x_dir - glb_usr.x_plane;

    float rayDirY0 = glb_usr.y_dir - glb_usr.y_plane;

    float rayDirX1 = glb_usr.x_dir + glb_usr.x_plane;

    float rayDirY1 = glb_usr.y_dir + glb_usr.y_plane;
    
    for (unsigned y = 0; y < glb_cfg.h; ++y) {		
	/* Horizontal distance from the camera to the floor for the current row. */
	/* 0.5 is the z position exactly in the middle between floor and ceiling. */

	float rowDistance = glb_usr.z_pos / (y - glb_usr.z_pos);
	
	/* Calculate the real world step vector we have to add for each x (parallel to camera plane) */
	/* Adding step by step avoids multiplications with a weight in the inner loop */

	float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / glb_cfg.w;

	float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / glb_cfg.w;
		
	/* Real world coordinates of the leftmost column. This will be updated as we step to the right. */

	float floorX = glb_usr.x_pos + rowDistance * rayDirX0;

	float floorY = glb_usr.y_pos + rowDistance * rayDirY0;
		
	for (unsigned x = 0; x < glb_cfg.w; ++x) {
	    int cellX = (int)floorX;

	    int cellY = (int)floorY;

	    SDL_Surface *real = getmtex(ftid(cellX, cellY));
	    
	    /* get the texture coordinate from the fractional part */

	    int texX = (int)(real->w * (floorX - cellX)) & (real->w - 1);

	    int texY = (int)(real->h * (floorY - cellY)) & (real->h - 1);
	    
	    floorX += floorStepX;

	    floorY += floorStepY;
            
	    SDL_Color rgb = getpixel(real, texX, texY);

	    drawpixel(glb_renderer, rgb, x, y);

	    if (glb_map.cfg.ceil) {
		real = getmtex(frid(cellX, cellY));
		
		rgb = getpixel(real, texX, texY);

		/* symmetrical, at screenHeight - y - 1 instead of y */
		    
		drawpixel(glb_renderer, rgb, x, glb_cfg.h - y - 1);
	    }
	}
    }

    return;
}

static void
rend_walls(void)
{
    for (unsigned x = 0; x < glb_cfg.w; ++x) {
	float cameraX = 2 * x / (float)glb_cfg.w - 1;

	float rayDirX = glb_usr.x_dir + glb_usr.x_plane * cameraX;

	float rayDirY = glb_usr.y_dir + glb_usr.y_plane * cameraX;
	
	/* Length of ray from one X or Y side to next X or Y side */

	float deltaDistX = fabs(1 / rayDirX);

	float deltaDistY = fabs(1 / rayDirY);

	/* What direction to step in X or Y direction (either +1 or -1) */

	char stepX = (rayDirX < 0) ?
	    -1 :
	    1 ;

	char stepY = (rayDirY < 0) ?
	    -1 :
	    1 ;

	/* Length of ray from current position to next X or Y side */
	
	float sideDistX = (rayDirX < 0) ?
	    (glb_usr.x_pos - uxpos) * deltaDistX :
	    (uxpos + 1.00 - glb_usr.x_pos) * deltaDistX ;

	float sideDistY = (rayDirY < 0) ?
	    (glb_usr.y_pos - uypos) * deltaDistY :
	    (uypos + 1.00 - glb_usr.y_pos) * deltaDistY ;

	unsigned mapX = uxpos;
	    
	unsigned mapY = uypos;

	/* Jump to next grid square, OR in X direction, OR in Y direction */

	/* tswitch flags which of the two map sections were hit */

	/* tile = 1; roof = -1; */
	    
	char tswitch = 0;

	bool side = false;
	
	while (!tswitch) {
	    if (sideDistX < sideDistY) {
		sideDistX += deltaDistX;

		mapX += stepX;

		side = false;
	    }
	    else {
		sideDistY += deltaDistY;

		mapY += stepY;

		side = true;
	    }

	    if (glb_map.cfg.tile)
		if (wtid(mapX, mapY))
		    tswitch = 1;

	    if (glb_map.cfg.roof)
		if (wrid(mapX, mapY))
		    tswitch = -1;
	}
	    
	/* Calculate distance projected on camera direction */

	float perpWallDist = (!side) ?
	    sideDistX - deltaDistX :
	    sideDistY - deltaDistY ;

	/* Calculate lowest and highest pixel to fill in current stripe */
	
	unsigned lineHeight = (unsigned)(glb_cfg.h / perpWallDist);

	unsigned drawStart = (-lineHeight + glb_cfg.h) / 2;

	if (drawStart > glb_cfg.h) drawStart = 0;
	
	unsigned drawEnd = (lineHeight + glb_cfg.h) / 2;

	if (drawEnd > glb_cfg.h) drawEnd = glb_cfg.h;
		
	float wallX = (!side) ?
	    glb_usr.y_pos + perpWallDist * rayDirY :
	    glb_usr.x_pos + perpWallDist * rayDirX ;

	wallX -= floor(wallX);

	SDL_Surface *real = getmtex(tswitch > 0 ? wtid(mapX, mapY) : wrid(mapX, mapY));

	int texX = (int)(wallX * (float)real->w);

	if ((side == 0 && rayDirX > 0) ||
	    (side == 1 && rayDirY < 0))
	    texX = real->w - texX - 1;

	float step = 1.00 * real->h / lineHeight;

	float tex_pos = (drawStart - glb_cfg.h / 2 + lineHeight / 2) * step;

	for (unsigned y = drawStart; y < drawEnd; ++y) {
	    int texY = (int)tex_pos & (real->h - 1);

	    tex_pos += step;

	    SDL_Color rgb = glb_map.cfg.fog ?
		interpolate_rgb(getpixel(real, texX, texY), glb_map.fog.c, perpWallDist / glb_map.fog.f) :
		getpixel(real, texX, texY);
	    
	    if (glb_map.cfg.shadows && side)
		{rgb.r /= 2; rgb.g /= 2; rgb.b /= 2;}

	    drawpixel(glb_renderer, rgb, x, y);
	}
	
	rend_z_buffer[x] = perpWallDist;
    }

    return;
}

static void
rend_sprites(void)
{
    int spriteOrder[glb_mspr.n];
    
    /* Sort sprites from far to close */

    for (unsigned i = 0; i < glb_mspr.n; ++i) {
        spriteOrder[i] = i;

	rend_spr_dist[i] = ((glb_usr.x_pos - glb_mspr.ref[i].x) *
			    (glb_usr.x_pos - glb_mspr.ref[i].x) +
			    (glb_usr.y_pos - glb_mspr.ref[i].y) *
			    (glb_usr.y_pos - glb_mspr.ref[i].y));
    }
    
    for (unsigned i = 0; i < glb_mspr.n; ++i) {
	/* Translate sprite position relative to camera */

	float spriteX = glb_mspr.ref[spriteOrder[i]].x - glb_usr.x_pos;

	float spriteY = glb_mspr.ref[spriteOrder[i]].y - glb_usr.y_pos;

	/* Transform sprite with inverse camera matrix */

	float invDet = 1.00 / (glb_usr.x_plane * glb_usr.y_dir - glb_usr.x_dir * glb_usr.y_plane); //Required for correct matrix multiplicaton

	float transformX = invDet * (glb_usr.y_dir * spriteX - glb_usr.x_dir * spriteY);

	float transformY = invDet * (-glb_usr.y_plane * spriteX + glb_usr.x_plane * spriteY); //The depth inside the screen, Z in 3D
        
	int spriteScreenX = (int)((glb_cfg.w / 2) * (1 + transformX / transformY));
        
	/* Calcluate height of sprite on screen */

	int spriteHeight = abs((int)(glb_cfg.h / transformY)); //Using transformY instead of real distance negates fisheye

	/* Calculate lowest and highest pixel to fill in stripe */

	int drawStartY = (-spriteHeight + glb_cfg.h) / 2;

	if (drawStartY < 0) drawStartY = 0;

	int drawEndY = (spriteHeight + glb_cfg.h) / 2;

	if (drawEndY >= (int)glb_cfg.h) drawEndY = glb_cfg.h;
        
	/* Calculate width of sprite TODO: Don't just use the width */

	int spriteWidth = spriteHeight;

	int drawStartX = -spriteWidth / 2 + spriteScreenX;

	if (drawStartX < 0) drawStartX = 0;

	int drawEndX = spriteWidth / 2 + spriteScreenX;

	if (drawEndX >= (int)glb_cfg.w) drawEndX = glb_cfg.w;

	SDL_Surface *real = getmtex(glb_mspr.ref[spriteOrder[i]].tex);
        
	for (int str = drawStartX; str < drawEndX; ++str) {
	    int texX = (int)(256 * (str - (-spriteWidth / 2 + spriteScreenX)) * real->w / spriteWidth) / 256;

	    /* Conditions:
	       1) it's in front of the camera plane
	       2) it's on the screen (left)
	       3) it's on the screen (right)
	       4) rend_z_buffer, with perpendicular distance */

	    if (transformY > 0
		&& str > 0
		&& str < (int)glb_cfg.w
		&& transformY < rend_z_buffer[str])
		for (int y = drawStartY; y < drawEndY; ++y) {
		    int d = y * 256 - glb_cfg.h * 128 + spriteHeight * 128; //256 and 128 factors to avoid floats

		    int texY = d * real->h / spriteHeight / 256;

		    SDL_Color rgb = getpixel(real, texX, texY);
                
		    if (rgb.r != 0x00 && rgb.g != 0x00 && rgb.b != 0x00)
			drawpixel(glb_renderer, rgb, str, y);
		}
	}
    }

    return;
}

void
get_rend(void)
{
    static bool mal = false;

    if (!mal) {
	rend_z_buffer = malloc(glb_cfg.w * sizeof(float));

	rend_spr_dist = malloc(glb_mspr.n * sizeof(float));

	mal = true;
    }
    
    SDL_RenderClear(glb_renderer);
    
    if (glb_map.cfg.floor)
	rend_floor();

    if (glb_map.cfg.walls)
	rend_walls();

    if (glb_map.cfg.sprites)
	rend_sprites();
    
    SDL_RenderPresent(glb_renderer);
    
    return;
}
