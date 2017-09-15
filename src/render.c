#include "render.h"
#include <stb_image.h>
#include <assert.h>
#include <string.h>
#include SHADER_INCLUDE_VS
#include SHADER_INCLUDE_FS
#include <entrypoint.h>
#include "filesystem.h"
#include "render_batch.h"
#include "_missing_texture.h"

r_color_t r_color(float r, float g, float b, float a)
{
	return r_coloru(
		(uint8_t)(gb_clamp01(r) * 255.0f),
		(uint8_t)(gb_clamp01(g) * 255.0f),
		(uint8_t)(gb_clamp01(b) * 255.0f),
		(uint8_t)(gb_clamp01(a) * 255.0f)
	);
}
r_colorf_t r_colorf(float r, float g, float b, float a)
{
	r_colorf_t ret;
	ret.r = r;
	ret.g = g;
	ret.b = b;
	ret.a = a;
	return ret;
}
r_color_t r_coloru(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	// assume little endian
	return ((r_color_t)a << 24) + ((r_color_t)b << 16) + ((r_color_t)g <<  8) + (r_color_t)r;
}
r_colorf_t r_colorfu(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return r_colorf((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f);
}
uint32_t r_color_to_rgba(r_color_t abgr_color)
{
	return ((abgr_color & 0x000000ff) << 24u) | ((abgr_color & 0x0000ff00) <<  8u) | ((abgr_color & 0x00ff0000) >>  8u) | ((abgr_color & 0xff000000) >> 24u);
}
r_colorf_t r_color_to_colorf(r_color_t abgr_color)
{
	return r_colorfu(abgr_color & 0xff, (abgr_color >> 8) & 0xff, (abgr_color >> 16) & 0xff, (abgr_color >> 24) & 0xff);
}
r_color_t r_colorf_to_color(r_colorf_t color)
{
	return r_color(color.r, color.g, color.b, color.a);
}
r_colorf_t r_colorf_mul(r_colorf_t a, r_colorf_t b)
{
	r_colorf_t ret;
	ret.r = a.r * b.r;
	ret.g = a.g * b.g;
	ret.b = a.b * b.b;
	ret.a = a.a * b.a;
	return ret;
}

static struct
{
	bgfx_vertex_decl_t			vert_decl;
	bgfx_uniform_handle_t		s_texture;
	bgfx_program_handle_t		prog;
	tex_t						white_tex;
	uint8_t						viewid;
	uint32_t					view_color;
	uint16_t					view_x, view_y, view_w, view_h;

	bool hint_no_alpha;
} ctx;

void _r_init()
{
	bgfx_vertex_decl_begin(&ctx.vert_decl, BGFX_RENDERER_TYPE_NOOP);
	bgfx_vertex_decl_add(&ctx.vert_decl, BGFX_ATTRIB_POSITION,  3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
	bgfx_vertex_decl_add(&ctx.vert_decl, BGFX_ATTRIB_TEXCOORD0, 2, BGFX_ATTRIB_TYPE_FLOAT, false, false);
	bgfx_vertex_decl_add(&ctx.vert_decl, BGFX_ATTRIB_COLOR0,    4, BGFX_ATTRIB_TYPE_UINT8,  true, false);
	bgfx_vertex_decl_end(&ctx.vert_decl);
	assert(ctx.vert_decl.stride == sizeof(vrtx_t));

	ctx.s_texture = bgfx_create_uniform("s_texture", BGFX_UNIFORM_TYPE_INT1, 1);

	// TODO support others
	bgfx_shader_handle_t vs = bgfx_create_shader(bgfx_make_ref(tex_color_vs, sizeof(tex_color_vs)));
	bgfx_shader_handle_t fs = bgfx_create_shader(bgfx_make_ref(tex_color_fs, sizeof(tex_color_fs)));
	ctx.prog = bgfx_create_program(vs, fs, true);

	static r_color_t white_color = 0xffffffff;
	ctx.white_tex.tex = bgfx_create_texture_2d(1, 1, false, 0, BGFX_TEXTURE_FORMAT_RGBA8, BGFX_TEXTURE_U_MIRROR | BGFX_TEXTURE_W_MIRROR | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIN_POINT, bgfx_make_ref(&white_color, sizeof(white_color)));
	ctx.white_tex.w = 1; ctx.white_tex.h = 1;
	ctx.white_tex.pixel_w = 1; ctx.white_tex.h = 1;
	ctx.white_tex.u1 = 0.0f; ctx.white_tex.v1 = 0.0f;
	ctx.white_tex.u2 = 1.0f; ctx.white_tex.v2 = 1.0f;

	rb_init();
}

void _r_deinit()
{
	rb_deinit();
	bgfx_destroy_texture(ctx.white_tex.tex);
	bgfx_destroy_program(ctx.prog);
	bgfx_destroy_uniform(ctx.s_texture);
}

static bool _ends_with(const char * str, const char * ext)
{
	size_t str_len = strlen(str);
	size_t suffix_len = strlen(ext);
	return (str_len >= suffix_len) && (!strcmp(str + (str_len - suffix_len), ext));
}

tex_t r_load(const char * filename, uint32_t flags)
{
	tex_t ret = {0};

	// TODO prefer loading ktx over other formats

	uint32_t tex_flags = BGFX_TEXTURE_NONE
			| BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP
			| (flags & TEX_FLAGS_POINT  ? (BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIN_POINT) : 0)
			| (flags & TEX_FLAGS_REPEAT ? (BGFX_TEXTURE_U_MIRROR | BGFX_TEXTURE_W_MIRROR) : 0);

	if(_ends_with(filename, ".ktx")) // native bgfx format
	{
		FILE * f = fsopen(filename, "rb");
		if(f)
		{
			fseek(f, 0, SEEK_END);
			const bgfx_memory_t * mem = bgfx_alloc(ftell(f));
			fseek(f, 0, SEEK_SET);
			fread(mem->data, mem->size, 1, f);
			fclose(f);

			bgfx_texture_info_t t;
			ret.tex = bgfx_create_texture(mem, tex_flags, 0, &t);
			ret.pixel_w = t.width;
			ret.pixel_h = t.height;
		}
	}
	else
	{
		int x = 0, y = 0, comp = 0;
		stbi_uc * bytes = stbi_fsload(filename, &x, &y, &comp, 4);

		if(bytes)
		{
			// TODO do we really need a copy here?
			// TODO generate mipmaps on a fly?

			ret.tex = bgfx_create_texture_2d(x, y, false, 1, BGFX_TEXTURE_FORMAT_RGBA8, tex_flags, bgfx_copy(bytes, x * y * 4));
			ret.pixel_w = x;
			ret.pixel_h = y;
			stbi_image_free(bytes);
		}
	}

	if(ret.tex.idx == 0 || ret.pixel_w == 0 || ret.pixel_h == 0)
	{
		fprintf(stderr, "failed to load %s\n", filename);

		bgfx_texture_info_t t;
		ret.tex = bgfx_create_texture(bgfx_make_ref(_missing_texture, sizeof(_missing_texture)), BGFX_TEXTURE_NONE, 0, &t);
		ret.pixel_w = t.width;
		ret.pixel_h = t.height;
		assert(ret.tex.idx && ret.pixel_w && ret.pixel_h);
	}

	ret.u1 = 0.0f; ret.v1 = 0.0f;
	ret.u2 = 1.0f; ret.v2 = 1.0f;
	ret.w = ret.pixel_w; ret.h = ret.pixel_h;

	return ret;
}

void r_free(tex_t tex)
{
	bgfx_destroy_texture(tex.tex);
}

static void r_setup_viewid(bool first)
{
	if(first)
		bgfx_set_view_clear(ctx.viewid, BGFX_CLEAR_COLOR, ctx.view_color, 0.0f, 0);
	else
		bgfx_set_view_clear(ctx.viewid, BGFX_CLEAR_NONE, 0, 0.0f, 0);

	float wf = (float)ctx.view_w / 2.0f, hf = (float)ctx.view_h / 2.0f;
	tr_set_view_prj(ctx.viewid, tr_ortho(-wf, wf, -hf, hf, -1.0f, 1.0f), tr_identity(), gb_vec2(ctx.view_x, ctx.view_y), gb_vec2(ctx.view_w, ctx.view_h));

	bgfx_touch(ctx.viewid);
	bgfx_set_view_mode(ctx.viewid, BGFX_VIEW_MODE_SEQUENTIAL);
}

static void r_next_viewid()
{
	rb_flush();
	++ctx.viewid;
	r_setup_viewid(false);
}

void r_viewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h, r_color_t color)
{
	ctx.view_color = r_color_to_rgba(color);
	ctx.view_x = x;
	ctx.view_y = y;
	ctx.view_w = w;
	ctx.view_h = h;

	ctx.viewid = 0;
	r_setup_viewid(true);
	tr_set_parent_world(tr_identity());
	rb_start();

	ctx.hint_no_alpha = false;
}

void r_frame_end()
{
	rb_flush();
}

void r_pixel_perfect_map(float * x, float * y, float w, float h)
{
	float vdx = (float)ctx.view_w / 2.0f;
	float vdy = (float)ctx.view_h / 2.0f;
	vdx -= (int)vdx;
	vdy -= (int)vdy;

	float dx = w * 0.5f - *x;
	float dy = h * 0.5f - *y;
	dx -= (int)dx;
	dy -= (int)dy;

	*x += vdx - dx;
	*y -= vdy - dy;
}

void r_render_hint_no_alpha()
{
	ctx.hint_no_alpha = true;
}

void r_render_sprite(tex_t tex, float x, float y, float r_deg, float sx, float sy)
{
	r_render_sprite_ex(tex, x, y, r_deg, 0.0f, 0.0f, sx, sy, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, false);
}

void r_render_sprite_ex(tex_t tex, float x, float y, float r_deg, float rox, float roy, float sx, float sy, float sox, float soy, float ox, float oy, float r, float g, float b, float a, bool pixel_perfect)
{
	if(pixel_perfect)
		r_pixel_perfect_map(&x, &y, tex.w * sx, tex.h * sy);

	tr_set_world(tr_model_spr(x, y, r_deg, rox, roy, sx, sy, sox, soy, tex.w, tex.h, ox, oy));

	r_color_t color = r_color(r, g, b, a);

	vrtx_t sprite_vertices[4] =
	{
		// 0 1
		// 3 2
		{-0.5f,  0.5f, 0.0f, tex.u1, tex.v1, color },
		{ 0.5f,  0.5f, 0.0f, tex.u2, tex.v1, color },
		{ 0.5f, -0.5f, 0.0f, tex.u2, tex.v2, color },
		{-0.5f, -0.5f, 0.0f, tex.u1, tex.v2, color },
	};

	uint16_t sprite_indices[6] =
	{
		0, 1, 2, // TODO resolve dat shit with ccw vs cw
		0, 2, 3,
	};

	uint64_t state = BGFX_STATE_DEFAULT_2D;

	if(!ctx.hint_no_alpha)
		state |= BGFX_STATE_BLEND_ALPHA;
	ctx.hint_no_alpha = false;

	r_render_transient(
		sprite_vertices, 4,
		sprite_indices, 6,
		tex.tex,
		1.0f, 1.0f, 1.0f, 1.0f,
		state
	);
}

void r_render_transient(
	vrtx_t * vbuf,
	uint16_t vbuf_count,
	uint16_t * ibuf,
	uint32_t ibuf_count,
	bgfx_texture_handle_t tex,
	float r, float g, float b, float a,
	uint64_t state
)
{
	// apply transform
	trns_t world = tr_get_world();
	for(size_t i = 0; i < vbuf_count; ++i)
	{
		gbVec4 out;
		gb_mat4_mul_vec4(&out, &world, gb_vec4(vbuf[i].x, vbuf[i].y, 0.0f, 1.0f));
		vbuf[i].x = out.x;
		vbuf[i].y = out.y;
	}

	// apply color
	if(r != 1.0f || g != 1.0f || b != 1.0f || a != 1.0f)
		for(size_t i = 0; i < vbuf_count; ++i)
			vbuf[i].color = r_colorf_to_color(r_colorf_mul(r_color_to_colorf(vbuf[i].color), r_colorf(r, g, b, a)));

	rb_add(tex, vbuf, vbuf_count, ibuf, ibuf_count, state);
}

void r_scissors(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	r_next_viewid();
	bgfx_set_view_scissor(ctx.viewid, x, y, w, h);
}

void r_scissors_clear()
{
	r_next_viewid();
	bgfx_set_view_scissor(ctx.viewid, 0, 0, 0, 0);
}

bgfx_vertex_decl_t *	r_decl()		{return &ctx.vert_decl;}
uint8_t					r_viewid()		{return ctx.viewid;}
bgfx_uniform_handle_t	r_s_texture()	{return ctx.s_texture;}
bgfx_program_handle_t	r_prog()		{return ctx.prog;}
tex_t					r_white_tex()	{return ctx.white_tex;}
