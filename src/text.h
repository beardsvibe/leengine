#pragma once

#include <stdint.h>
#include <stdbool.h>

void _t_init(uint32_t w, uint32_t h); // size of text texture atlas in pixels
void _t_deinit();

int32_t t_add(const char * fontname, const char * filename);

#define TEXT_ALIGN_LEFT		(1 << 0) // horizontal align default
#define TEXT_ALIGN_CENTER	(1 << 1)
#define TEXT_ALIGN_RIGHT	(1 << 2)
#define TEXT_ALIGN_TOP		(1 << 3)
#define TEXT_ALIGN_MIDDLE	(1 << 4)
#define TEXT_ALIGN_BOTTOM	(1 << 5)
#define TEXT_ALIGN_BASELINE	(1 << 6) // vertical align default

void r_text_ex2(int32_t font,
				float x, float y, float deg, float sx, float sy, float ox, float oy, float r, float g, float b, float a,
				float * out_bounds, uint8_t align, float size_in_pt, float spacing_in_pt,
				const char * text, ...);

void _r_text_debug_atlas(float k_size, bool blend);
