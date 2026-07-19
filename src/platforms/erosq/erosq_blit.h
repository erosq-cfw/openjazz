#ifndef EROSQ_BLIT_H
#define EROSQ_BLIT_H
#include <stdint.h>
#include <stddef.h>

/* Pure, host-testable: expand src_w*src_h 8-bit indices into an RGB565 dst
 * starting at row y_off, via a 256-entry LUT (low 16 bits used). No SDL/drm
 * deps so it can be unit-tested on the host. */
#ifdef __cplusplus
extern "C" {
#endif
void erosq_expand565(uint16_t *dst, int dst_stride_px, const uint8_t *src,
                     int src_w, int src_h, int y_off, const uint32_t *lut565);
#ifdef __cplusplus
}
#endif

#endif
