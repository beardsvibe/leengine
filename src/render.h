#pragma once

#include <stdint.h>
#include <bgfx.h>
#include "transforms.h"

typedef uint32_t r_color_t;

static inline r_color_t r_to_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	// assume little endian
	return ((r_color_t)a << 24) + ((r_color_t)b << 16) + ((r_color_t)g <<  8) + (r_color_t)r;
}

// default vertex
#pragma pack(push, 1)
typedef struct
{
	float x, y, z;
	float u, v;
	r_color_t color;
} vrtx_t;
#pragma pack(pop)

// texture structure
typedef struct
{
	bgfx_texture_handle_t tex;
	uint16_t w, h;
} tex_t;

// default 2d rendering state
#define BGFX_STATE_DEFAULT_2D (0 \
	| BGFX_STATE_RGB_WRITE \
	| BGFX_STATE_ALPHA_WRITE \
	| BGFX_STATE_CULL_CCW \
	| BGFX_STATE_MSAA \
)

// texture loading flags
#define TEX_FLAGS_NONE		0x0
#define TEX_FLAGS_POINT		0x1 // disables filtering
#define TEX_FLAGS_REPEAT	0x2

void	_r_init();
void	_r_deinit();

tex_t	r_load(const char * filename, uint32_t flags);
void	r_free(tex_t tex);

void	r_viewport(uint16_t w, uint16_t h);
void	r_render_ex(tex_t tex, float x, float y, float deg, float sx, float sy);
void	r_render(tex_t tex, float x, float y, float deg);

bgfx_vertex_decl_t *	r_decl();	// vertex declaration
bgfx_uniform_handle_t	r_u_tex();	// texture uniform
bgfx_program_handle_t	r_prog();	// program
tex_t					r_white_tex(); // white texture
