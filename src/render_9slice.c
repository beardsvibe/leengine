
#include "render_9slice.h"

static float _u(tex_t tex, float u) {return u * (tex.u2 - tex.u1) + tex.u1;}
static float _v(tex_t tex, float v) {return v * (tex.v2 - tex.v1) + tex.v1;}

void r_9slice(
	tex_t tex, tex_9slice_t slice,
	float w, float h,
	float x, float y,
	float r_deg, float rox, float roy,
	float ox, float oy,
	float r, float g, float b, float a,
	bool pixel_perfect)
{
	// 0----1---------------2----3
	// |    |               |    |
	// 4----5---------------6----7
	// |    |               |    |
	// |    |               |    |
	// |    |               |    |
	// 8----9--------------10---11
	// |    |               |    |
	// |    |               |    |
	// 12--13--------------14---15

	vrtx_t vert[16];

	float arr_u[4] = {0.0f, slice.p1u, slice.p2u, 1.0f};
	float arr_v[4] = {0.0f, slice.p1v, slice.p2v, 1.0f};

	float lw = (_u(tex, arr_u[1]) - _u(tex, arr_u[0])) * ((float)tex.pixel_w);
	float rw = (_u(tex, arr_u[3]) - _u(tex, arr_u[2])) * ((float)tex.pixel_w);
	float th = (_v(tex, arr_v[1]) - _v(tex, arr_v[0])) * ((float)tex.pixel_h);
	float bh = (_v(tex, arr_v[3]) - _v(tex, arr_v[2])) * ((float)tex.pixel_h);

	float kw = w / (lw + rw);
	float kh = h / (th + bh);
	float k = (kw < 1.0f || kh < 1.0f) ? (kw < kh ? kw : kh) : 1.0f;
	k *= slice.scale;

	float arr_x[4] = {-w / 2.0f, -w / 2.0f + lw * k,  w / 2.0f - rw * k,  w / 2.0f};
	float arr_y[4] = { h / 2.0f,  h / 2.0f - th * k, -h / 2.0f + bh * k, -h / 2.0f};

	for(uint16_t i = 0; i < 16; ++i)
	{
		vert[i].x = arr_x[i % 4] / w;
		vert[i].y = arr_y[i / 4] / h;
		vert[i].z = 0.0f;
		vert[i].u = _u(tex, arr_u[i % 4]);
		vert[i].v = _v(tex, arr_v[i / 4]);
		vert[i].color = 0xffffffff;
	}

	uint16_t id[6 * 9];

	for(uint16_t i = 0; i < 9; ++i) // we only have 9 squares with 2 triangles in each
	{
		uint16_t base = (i / 3) + i;
		id[i * 6 + 0] = base + 0;
		id[i * 6 + 1] = base + 1;
		id[i * 6 + 2] = base + 5;
		id[i * 6 + 3] = base + 0;
		id[i * 6 + 4] = base + 5;
		id[i * 6 + 5] = base + 4;
	}

	if(pixel_perfect)
		r_pixel_perfect_map(&x, &y, w, h);
	tr_set_world(tr_model_spr(x, y, r_deg, rox, roy, 1.0f, 1.0f, 0.0f, 0.0f, w, h, ox, oy));
	r_render_transient(vert, 16, id, 6 * 9, tex.tex, r, g, b, a, BGFX_STATE_DEFAULT_2D | BGFX_STATE_BLEND_ALPHA);
}
