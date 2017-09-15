#pragma once

#include "render.h"

// |-------------------------|
// |    |               |    |
// |---p1---------------|----|
// |    |               |    |
// |    |               |    |
// |    |               |    |
// |----|--------------p2----|
// |    |               |    |
// |    |               |    |
// |-------------------------|

typedef struct
{
	float p1u, p1v, p2u, p2v;
	float scale;
} tex_9slice_t;

void r_9slice(tex_t tex, tex_9slice_t slice,
	float w, float h,
	float x, float y,
	float r_deg, float rox, float roy,
	float ox, float oy,
	float r, float g, float b, float a,
	bool pixel_perfect);
