#include "erosq_video.h"
#include "erosq_blit.h"
#include "drm_out.h"
#include <string.h>

/* RGB565 LUT rebuilt from the current palette each frame (256 entries — cheap,
 * and dodges tracking palette-effect dirty state). ponytail: per-frame rebuild,
 * revisit only if it ever shows on a profile (it won't at 320x200). */
static uint32_t lut565[256];

int erosq_video_init(void)
{
	return drm_open();
}

void erosq_video_deinit(void)
{
	drm_close();
}

void erosq_present(const void *pixels, const SDL_Color *palette, int w, int h)
{
	int stride;
	uint16_t *fb = drm_pixels(&stride);
	if (!fb)
		return;

	for (int i = 0; i < 256; ++i) {
		unsigned r = palette[i].r, g = palette[i].g, b = palette[i].b;
#ifdef EROSQ_BGR565
		lut565[i] = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);
#else
		lut565[i] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
#endif
	}

	/* Clear the whole frame once so the letterbox bars stay black; the game
	 * area (rows y_off..y_off+h) is overwritten every frame below. */
	static int bars_cleared = 0;
	int y_off = (DRM_OUT_H - h) / 2;   /* (240-200)/2 = 20 */
	if (!bars_cleared) {
		for (int y = 0; y < DRM_OUT_H; ++y)
			memset(fb + (size_t)y * stride, 0, (size_t)DRM_OUT_W * 2);
		bars_cleared = 1;
	}

	erosq_expand565(fb, stride, (const uint8_t *)pixels, w, h, y_off, lut565);
	drm_present();
}
