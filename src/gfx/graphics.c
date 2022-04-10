#include <SDL2/sdl.h>
#include "gfx/graphics.h"

static Uint32
getrawpixel(SDL_Surface *surface, int x, int y)
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
getpixel(SDL_Surface *surface, int x, int y)
{
    SDL_Color rgb;

    SDL_GetRGB(getrawpixel(surface, x, y), surface->format, &rgb.r, &rgb.g, &rgb.b);

    return rgb;
}

static void
drawpixel(SDL_Renderer *renderer, SDL_Color color, int x, int y)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);

    SDL_RenderDrawPoint(renderer, x, y);

    return;
}

static inline SDL_Color
interpolate_rgb(SDL_Color rgb, SDL_Color c, double fpcnt) /* Here fpcnt should be perpWallDist / fog.f */
{
    return (SDL_Color) {
	(1 - (fpcnt > 1) ? 1 : fpcnt) * rgb.r + fpcnt * c.r,
	(1 - (fpcnt > 1) ? 1 : fpcnt) * rgb.g + fpcnt * c.g,
	(1 - (fpcnt > 1) ? 1 : fpcnt) * rgb.b + fpcnt * c.b,
	SDL_ALPHA_OPAQUE
    };
}
