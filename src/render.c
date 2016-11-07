#include "render.h"
#include <stb_image.h>
#include <assert.h>
#include <string.h>
#include <tex_color_vs.h>
#include <tex_color_fs.h>
#include <entrypoint.h>
#include "filesystem.h"
#include "_missing_texture.h"

static vrtx_t sprite_vertices[4] =
{
	// 0 1
	// 3 2
	{-0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0xffffffff },
	{ 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0xffffffff },
	{ 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0xffffffff },
	{-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0xffffffff },
};

static const uint16_t sprite_indices[6] =
{
	0, 1, 2, // TODO resolve dat shit with ccw vs cw
	0, 2, 3,
};

static struct
{
	bgfx_vertex_decl_t			vert_decl;
	bgfx_uniform_handle_t		s_texture;
	bgfx_uniform_handle_t		u_diffuse;
	bgfx_program_handle_t		prog;
	bgfx_vertex_buffer_handle_t	v_buf;
	bgfx_index_buffer_handle_t	i_buf;
	tex_t						white_tex;
} ctx;

void _r_init()
{
	bgfx_vertex_decl_begin(&ctx.vert_decl, BGFX_RENDERER_TYPE_NOOP);
	bgfx_vertex_decl_add(&ctx.vert_decl, BGFX_ATTRIB_POSITION,  3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
	bgfx_vertex_decl_add(&ctx.vert_decl, BGFX_ATTRIB_TEXCOORD0, 2, BGFX_ATTRIB_TYPE_FLOAT, false, false);
	bgfx_vertex_decl_add(&ctx.vert_decl, BGFX_ATTRIB_COLOR0,    4, BGFX_ATTRIB_TYPE_UINT8,  true, false);
	bgfx_vertex_decl_end(&ctx.vert_decl);

	ctx.s_texture = bgfx_create_uniform("s_texture", BGFX_UNIFORM_TYPE_INT1, 1);
	ctx.u_diffuse = bgfx_create_uniform("u_diffuse", BGFX_UNIFORM_TYPE_VEC4, 1);

	// TODO support others
	bgfx_shader_handle_t vs = bgfx_create_shader(bgfx_make_ref(tex_color_vs, sizeof(tex_color_vs)));
	bgfx_shader_handle_t fs = bgfx_create_shader(bgfx_make_ref(tex_color_fs, sizeof(tex_color_fs)));
	ctx.prog = bgfx_create_program(vs, fs, true);

	ctx.v_buf = bgfx_create_vertex_buffer(bgfx_make_ref(sprite_vertices, sizeof(sprite_vertices)), &ctx.vert_decl, BGFX_BUFFER_NONE);
	ctx.i_buf = bgfx_create_index_buffer(bgfx_make_ref(sprite_indices, sizeof(sprite_indices)), BGFX_BUFFER_NONE);

	static r_color_t white_color = 0xffffffff;
	ctx.white_tex.tex = bgfx_create_texture_2d(1, 1, false, 0, BGFX_TEXTURE_FORMAT_RGBA8, BGFX_TEXTURE_U_MIRROR | BGFX_TEXTURE_W_MIRROR | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIN_POINT, bgfx_make_ref(&white_color, sizeof(white_color)));
	ctx.white_tex.w = 1;
	ctx.white_tex.h = 1;
}

void _r_deinit()
{
	bgfx_destroy_texture(ctx.white_tex.tex);
	bgfx_destroy_program(ctx.prog);
	bgfx_destroy_uniform(ctx.s_texture);
	bgfx_destroy_vertex_buffer(ctx.v_buf);
	bgfx_destroy_index_buffer(ctx.i_buf);
}

static bool _ends_with(const char * str, const char * ext)
{
	size_t str_len = strlen(str);
	size_t suffix_len = strlen(ext);
	return (str_len >= suffix_len) && (!strcmp(str + (str_len - suffix_len), ext));
}

tex_t r_load(const char * filename, uint32_t flags)
{
	(void)flags;

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
			ret.w = t.width;
			ret.h = t.height;
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
			ret.w = x;
			ret.h = y;
			stbi_image_free(bytes);
		}
	}

	if(ret.tex.idx == 0 || ret.w == 0 || ret.h == 0)
	{
		fprintf(stderr, "failed to load %s\n", filename);

		bgfx_texture_info_t t;
		ret.tex = bgfx_create_texture(bgfx_make_ref(_missing_texture, sizeof(_missing_texture)), BGFX_TEXTURE_NONE, 0, &t);
		ret.w = t.width;
		ret.h = t.height;

		assert(ret.tex.idx && ret.w && ret.h);
	}

	return ret;
}

void r_free(tex_t tex)
{
	bgfx_destroy_texture(tex.tex);
}

void r_viewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h, r_color_t color)
{
	bgfx_set_view_clear(0, BGFX_CLEAR_COLOR, r_color_to_rgba(color), 0.0f, 0);

	float wf = w / 2.0f, hf = h / 2.0f;
	tr_set_view_prj(0, tr_ortho(-wf, wf, -hf, hf, -1.0f, 1.0f), tr_identity(), gb_vec2(x, y), gb_vec2(w, h));

	tr_set_parent_world(tr_identity());
	bgfx_set_view_seq(0, true);
	bgfx_touch(0);
}

void r_render(tex_t tex, float x, float y, float deg)
{
	r_render_ex2(tex, x, y, deg, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
}

void r_render_ex(tex_t tex, float x, float y, float deg, float sx, float sy, float ox, float oy)
{
	r_render_ex2(tex, x, y, deg, sx, sy, ox, oy, 1.0f, 1.0f, 1.0f, 1.0f);
}

void r_render_ex2(tex_t tex, float x, float y, float deg, float sx, float sy, float ox, float oy, float r, float g, float b, float a)
{
	tr_set_world(tr_model_spr(x, y, deg, sx, sy, ox, oy, tex.w, tex.h));
	r_submit(ctx.v_buf, ctx.i_buf, tex.tex, r, g, b, a, BGFX_STATE_DEFAULT_2D | BGFX_STATE_BLEND_ALPHA);
}

void r_submit(
	bgfx_vertex_buffer_handle_t vbuf,
	bgfx_index_buffer_handle_t ibuf,
	bgfx_texture_handle_t tex,
	float diffuse_r, float diffuse_g, float diffuse_b, float diffuse_a,
	uint64_t state
)
{
	float diffuse[4] = {diffuse_r, diffuse_g, diffuse_b, diffuse_a};
	bgfx_set_vertex_buffer(vbuf, 0, -1);
	bgfx_set_index_buffer(ibuf, 0, -1);
	bgfx_set_texture(0, ctx.s_texture, tex, -1);
	bgfx_set_uniform(ctx.u_diffuse, diffuse, 1);
	bgfx_set_state(state, 0);
	bgfx_submit(0, ctx.prog, 0, false);
}
void r_submit_transient(
	bgfx_transient_vertex_buffer_t * vbuf,
	bgfx_transient_index_buffer_t * ibuf,
	bgfx_texture_handle_t tex,
	float diffuse_r, float diffuse_g, float diffuse_b, float diffuse_a,
	uint64_t state
)
{
	float diffuse[4] = {diffuse_r, diffuse_g, diffuse_b, diffuse_a};
	bgfx_set_transient_vertex_buffer(vbuf, 0, -1);
	bgfx_set_transient_index_buffer(ibuf, 0, -1);
	bgfx_set_texture(0, ctx.s_texture, tex, -1);
	bgfx_set_uniform(ctx.u_diffuse, diffuse, 1);
	bgfx_set_state(state, 0);
	bgfx_submit(0, ctx.prog, 0, false);
}

bgfx_vertex_decl_t *	r_decl()		{return &ctx.vert_decl;}
bgfx_uniform_handle_t	r_s_texture()	{return ctx.s_texture;}
bgfx_uniform_handle_t	r_u_diffuse()	{return ctx.u_diffuse;}
bgfx_program_handle_t	r_prog()		{return ctx.prog;}
tex_t					r_white_tex()	{return ctx.white_tex;}

bgfx_vertex_buffer_handle_t _r_sprvbuf() {return ctx.v_buf;}
bgfx_index_buffer_handle_t _r_spribuf() {return ctx.i_buf;}
