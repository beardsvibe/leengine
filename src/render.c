#include "render.h"
#include <stb_image.h>
#include <assert.h>
#include <string.h>
#include <tex_color_vs.h>
#include <tex_color_fs.h>
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
	bgfx_uniform_handle_t		u_tex;
	bgfx_program_handle_t		prog;
	bgfx_vertex_buffer_handle_t	v_buf;
	bgfx_index_buffer_handle_t	i_buf;
	tex_t						white_tex;
} ctx;

void _r_init()
{
	bgfx_vertex_decl_begin(&ctx.vert_decl, BGFX_RENDERER_TYPE_NULL);
	bgfx_vertex_decl_add(&ctx.vert_decl, BGFX_ATTRIB_POSITION,  3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
	bgfx_vertex_decl_add(&ctx.vert_decl, BGFX_ATTRIB_TEXCOORD0, 2, BGFX_ATTRIB_TYPE_FLOAT, false, false);
	bgfx_vertex_decl_add(&ctx.vert_decl, BGFX_ATTRIB_COLOR0,    4, BGFX_ATTRIB_TYPE_UINT8,  true, false);
	bgfx_vertex_decl_end(&ctx.vert_decl);

	ctx.u_tex = bgfx_create_uniform("texture", BGFX_UNIFORM_TYPE_INT1, 1);

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
	bgfx_destroy_uniform(ctx.u_tex);
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
			| (flags & TEX_FLAGS_POINT  ? (BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIN_POINT) : 0)
			| (flags & TEX_FLAGS_REPEAT ? (BGFX_TEXTURE_U_MIRROR | BGFX_TEXTURE_W_MIRROR) : 0);

	if(_ends_with(filename, ".ktx")) // native bgfx format
	{
		FILE * f = fopen(filename, "rb");
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
	#ifdef STB_IMAGE_AVAILABLE
	else
	{
		int x = 0, y = 0, comp = 0;
		stbi_uc * bytes = stbi_load(filename, &x, &y, &comp, 4);

		if(bytes)
		{
			// TODO do we really need a copy here?
			// TODO generate mipmaps on a fly?

			ret.tex = bgfx_create_texture_2d(x, y, false, 0, BGFX_TEXTURE_FORMAT_RGBA8, tex_flags, bgfx_copy(bytes, x * y * 4));
			ret.w = x;
			ret.h = y;
			stbi_image_free(bytes);
		}
	}
	#endif

	if(ret.tex.idx == 0 || ret.w == 0 || ret.h == 0)
	{
		fprintf(stderr, "failed to load %s\n", filename);

		bgfx_texture_info_t t;
		ret.tex = bgfx_create_texture(bgfx_make_ref(_missing_texture, _missing_texture_size), BGFX_TEXTURE_NONE, 0, &t);
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

void r_viewport(uint16_t w, uint16_t h)
{
	bgfx_set_view_clear(0, BGFX_CLEAR_COLOR, 0x202020ff, 0.0f, 0);
	bgfx_set_view_rect(0, 0, 0, w, h);
	bgfx_touch(0);

	float wf = w / 2.0f, hf = h / 2.0f;
	tr_set_mats(0, tr_ortho(-wf, wf, -hf, hf, -1000.0f, 1000.0f), tr_identity());

	bgfx_set_view_seq(0, true);
}

void r_render_ex(tex_t tex, float x, float y, float deg, float sx, float sy)
{
	float pos[3] = {x, y, 0.0f};
	float rot[3] = {0.0f, 0.0f, -deg};
	float scl[3] = {sx * tex.w, sy * tex.h, 1.0f};
	tr_set_world(tr_model(pos, rot, scl));

	bgfx_set_vertex_buffer(ctx.v_buf, 0, -1);
	bgfx_set_index_buffer(ctx.i_buf, 0, -1);
	bgfx_set_texture(0, r_u_tex(), tex.tex, -1);
	bgfx_set_state(BGFX_STATE_DEFAULT_2D | BGFX_STATE_BLEND_ALPHA, 0);
	bgfx_submit(0, r_prog(), 0, false);
}

void r_render(tex_t tex, float x, float y, float deg)
{
	r_render_ex(tex, x, y, deg, 1.0f, 1.0f);
}

bgfx_vertex_decl_t *	r_decl()	{return &ctx.vert_decl;}
bgfx_uniform_handle_t	r_u_tex()	{return ctx.u_tex;}
bgfx_program_handle_t	r_prog()	{return ctx.prog;}
tex_t					r_white_tex() {return ctx.white_tex;}
