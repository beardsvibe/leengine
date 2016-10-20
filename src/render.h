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

void	r_viewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void	r_render(tex_t tex, float x, float y, float deg);
void	r_render_ex(tex_t tex, float x, float y, float deg, float sx, float sy, float ox, float oy);
void	r_render_ex2(tex_t tex, float x, float y, float deg, float sx, float sy, float ox, float oy, float r, float g, float b, float a);

void	r_submit( // submit with default program
	bgfx_vertex_buffer_handle_t vbuf,
	bgfx_index_buffer_handle_t ibuf,
	bgfx_texture_handle_t tex,
	float diffuse_r, float diffuse_g, float diffuse_b, float diffuse_a,
	uint64_t state
);
void	r_submit_transient( // submit transient buffers with default program
	bgfx_transient_vertex_buffer_t * vbuf,
	bgfx_transient_index_buffer_t * ibuf,
	bgfx_texture_handle_t tex,
	float diffuse_r, float diffuse_g, float diffuse_b, float diffuse_a,
	uint64_t state
);


bgfx_vertex_decl_t *	r_decl();		// vertex declaration
bgfx_uniform_handle_t	r_s_texture();	// texture sampler uniform
bgfx_uniform_handle_t	r_u_diffuse();	// diffuse color uniform
bgfx_program_handle_t	r_prog();		// program
tex_t					r_white_tex();	// white texture
