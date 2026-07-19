#ifndef EROSQ_VIDEO_H
#define EROSQ_VIDEO_H

/* EROS Q panel output: expand OpenJazz's 8-bit indexed canvas into the drm_out
 * RGB565 scanout, letterboxed 320x200 -> 320x240. Bypasses SDL video. */

#include "SDL.h"  /* SDL_Color */

int  erosq_video_init(void);    /* open DRM output; 0 ok, -1 fail */
void erosq_video_deinit(void);

/* Present a w*h 8-bit indexed frame with the given 256-colour palette. */
void erosq_present(const void *pixels, const SDL_Color *palette, int w, int h);

#endif
