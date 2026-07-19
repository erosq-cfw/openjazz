#include "erosq_blit.h"

/* Expand 8-bit indexed src into RGB565 dst at row offset y_off.
 * Two pixels per 32-bit store: the scanout dumb buffer is uncached, so a
 * 16-bit store costs a full bus cycle -- pack pairs. (ponytail: naive per-row
 * loop; 320x200=64k gathers/frame is well under the frame budget.) */
void erosq_expand565(uint16_t *dst, int dst_stride_px, const uint8_t *src,
                     int src_w, int src_h, int y_off, const uint32_t *lut565)
{
	for (int y = 0; y < src_h; ++y)
	{
		uint16_t *d = dst + (size_t)(y + y_off) * dst_stride_px;
		const uint8_t *s = src + (size_t)y * src_w;
		int x = 0;
		for (; x + 1 < src_w; x += 2)
		{
			uint32_t two = (uint32_t)(uint16_t)lut565[s[x]]
			             | ((uint32_t)(uint16_t)lut565[s[x + 1]] << 16);
			*(uint32_t *)(d + x) = two;
		}
		for (; x < src_w; ++x)
			d[x] = (uint16_t)lut565[s[x]];
	}
}
