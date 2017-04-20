#pragma once

#include <stdint.h>
#include <bgfx.h>
#include "transforms.h"

// color stuff
typedef uint32_t r_color_t; // r_colot_t is ABGR
typedef struct {float r,g,b,a;} r_colorf_t;
r_color_t	r_color(float r, float g, float b, float a);
r_colorf_t	r_colorf(float r, float g, float b, float a);
r_color_t	r_coloru(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
r_colorf_t	r_colorfu(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t	r_color_to_rgba(r_color_t abgr_color);
r_colorf_t	r_color_to_colorf(r_color_t abgr_color);
r_color_t	r_colorf_to_color(r_colorf_t color);

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
	bgfx_texture_handle_t tex;	// texture handle
	uint16_t pixel_w, pixel_h;	// size of texture in pixels, please don't override
	float w, h;					// size of sprite, might be overrided externally
	float u1, v1, u2, v2;		// texture coordinates, might be overrided externally
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

void	r_viewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h, r_color_t color);
void	r_render(tex_t tex, float x, float y, float r_deg, float sx, float sy);
void	r_render_ex(tex_t tex, float x, float y, float r_deg, float rox, float roy, float sx, float sy, float sox, float soy);
void	r_render_ex2(tex_t tex, float x, float y, float r_deg, float rox, float roy, float sx, float sy, float sox, float soy, float ox, float oy, float r, float g, float b, float a);

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

void r_scissors(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void r_scissors_clear();

bgfx_vertex_decl_t *	r_decl();		// vertex declaration
bgfx_uniform_handle_t	r_s_texture();	// texture sampler uniform
bgfx_uniform_handle_t	r_u_diffuse();	// diffuse color uniform
bgfx_program_handle_t	r_prog();		// program
tex_t					r_white_tex();	// white texture

bgfx_vertex_buffer_handle_t _r_sprvbuf();
bgfx_index_buffer_handle_t _r_spribuf();
