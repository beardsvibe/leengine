
#include "render_text.h"
#include "render.h"
#include "filesystem.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#define FONTSTASH_IMPLEMENTATION
#include <fontstash.h>

#if !defined(EMSCRIPTEN) && !defined(__ANDROID__)
	#define NF // support native fonts
#endif

#ifdef NF
#include <nativefonts.h>
#include <khash.h>

typedef struct
{
	bgfx_texture_handle_t tex;
	uint16_t w, h;
	nf_aabb_t aabb;
	uint8_t ttl; // in frames
} nf_tex_t;

KHASH_MAP_INIT_STR(nf_text_map, nf_tex_t)

#endif

static struct
{
	FONScontext * fons;
	bool tex_valid;
	bgfx_texture_handle_t tex;
	uint32_t tex_w, tex_h;

#ifdef NF
	nf_font_t nf_font;
	kh_nf_text_map_t * nf_map;
#endif
} ctx = {0};

static void _tex_delete(void * userptr)
{
	if(ctx.tex_valid)
	{
		bgfx_destroy_texture(ctx.tex);
		ctx.tex_valid = false;
	}
}

static int _tex_resize(void * userptr, int width, int height)
{
	if(ctx.tex_valid)
		_tex_delete(userptr);
	ctx.tex = bgfx_create_texture_2d(width, height, false, 1, BGFX_TEXTURE_FORMAT_RGBA8, BGFX_TEXTURE_NONE, NULL);
	ctx.tex_w = width;
	ctx.tex_h = height;
	ctx.tex_valid = true;
	return 1;
}

static int _tex_create(void * userptr, int width, int height) {return _tex_resize(userptr, width, height);}

static void _tex_update(void * userptr, int* rect, const unsigned char* data)
{
	if(!ctx.tex_valid)
		return;

	uint32_t w = rect[2] - rect[0];
	uint32_t h = rect[3] - rect[1];
	uint32_t x = rect[0];
	uint32_t y = rect[1];

	const bgfx_memory_t * mem = bgfx_alloc(w * h * 4);
	for(size_t j = 0; j < h; ++j)
		for(size_t i = 0; i < w; ++i)
		{
			uint8_t c = data[(j + y) * ctx.tex_w + i + x];
			((r_color_t*)mem->data)[j * w + i] = r_coloru(255, 255, 255, c);
		}
	bgfx_update_texture_2d(ctx.tex, 0, 0, x, y, w, h, mem, -1);
}

static void _draw(void * userptr, const float* verts, const float* tcoords, const unsigned int* colors, int nverts)
{
	if(!ctx.tex_valid)
		return;

	bgfx_transient_vertex_buffer_t vt;
	bgfx_alloc_transient_vertex_buffer(&vt, nverts, r_decl());
	vrtx_t * vert = (vrtx_t*)vt.data;
	for(size_t i = 0; i < nverts; ++i)
	{
		vert[i].x = verts[i * 2 + 0];
		vert[i].y = -verts[i * 2 + 1];
		vert[i].z = 0.0f;
		vert[i].u = tcoords[i * 2 + 0];
		vert[i].v = tcoords[i * 2 + 1];
		vert[i].color = colors[i];
	}

	bgfx_transient_index_buffer_t it;
	bgfx_alloc_transient_index_buffer(&it, nverts);
	uint16_t * id = (uint16_t*)it.data;
	for(size_t i = 0; i < nverts / 3; ++i)
	{
		id[i * 3 + 0] = (uint16_t)(i * 3 + 0);
		id[i * 3 + 1] = (uint16_t)(i * 3 + 2);
		id[i * 3 + 2] = (uint16_t)(i * 3 + 1);
	}

	r_submit_transient(&vt, &it, ctx.tex, 1.0f, 1.0f, 1.0f, 1.0f, BGFX_STATE_DEFAULT_2D | BGFX_STATE_BLEND_ALPHA);
}

void _t_init(uint32_t w, uint32_t h)
{
	FONSparams params;
	memset(&params, 0, sizeof(params));
	params.width = w;
	params.height = h;
	params.flags = (unsigned char)FONS_ZERO_TOPLEFT;
	params.renderCreate = _tex_create;
	params.renderResize = _tex_resize;
	params.renderUpdate = _tex_update;
	params.renderDraw = _draw;
	params.renderDelete = _tex_delete;
	params.userPtr = NULL;
	ctx.fons = fonsCreateInternal(&params);

#ifdef NF
	nf_font_params_t nf_params = {0};
	nf_params.size_in_pt = 32.0f;
	ctx.nf_font = nf_font(NULL, nf_params);
	ctx.nf_map = kh_init_nf_text_map();
#endif
}

void _t_deinit()
{
#ifdef NF
	for(khint_t k = kh_begin(ctx.nf_map); k != kh_end(ctx.nf_map); ++k)
	{
		if(kh_exist(ctx.nf_map, k))
		{
			free((char*)kh_key(ctx.nf_map, k));
			bgfx_destroy_texture(kh_value(ctx.nf_map, k).tex);
		}
	}
	kh_clear_nf_text_map(ctx.nf_map);
	kh_destroy_nf_text_map(ctx.nf_map);
	nf_free(ctx.nf_font);
#endif

	fonsDeleteInternal(ctx.fons);
}

void _t_cleanup()
{
#ifdef NF
	for(khint_t k = kh_begin(ctx.nf_map); k != kh_end(ctx.nf_map); ++k)
	{
		if(!kh_exist(ctx.nf_map, k))
			continue;

		--kh_value(ctx.nf_map, k).ttl;
		if(!kh_value(ctx.nf_map, k).ttl)
		{
			free((char*)kh_key(ctx.nf_map, k));
			bgfx_destroy_texture(kh_value(ctx.nf_map, k).tex);
			kh_del_nf_text_map(ctx.nf_map, k);
		}
	}

//	printf("size %i\n", kh_size(ctx.nf_map));

#endif
}

font_t t_add(const char * fontname, const char * filename)
{
	int res = fonsGetFontByName(ctx.fons, fontname);
	if(res != FONS_INVALID)
		return res;

	FILE * fp = fsopen(filename, "rb");
	if(!fp)
		return FONS_INVALID;

	fseek(fp,0,SEEK_END);
	size_t size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	if(!size)
	{
		fclose(fp);
		return FONS_INVALID;
	}

	uint8_t * data = (uint8_t*)malloc(size);
	fread(data, 1, size, fp);
	fclose(fp);

	return fonsAddFontMem(ctx.fons, fontname, data, (int)size, 1);
}

#ifdef NF // nativefonts rendering
static void _r_nf(float r, float g, float b, float a, float * out_bounds, uint8_t align, const char * text)
{
	khint_t k = kh_get_nf_text_map(ctx.nf_map, text);

	if(k == kh_end(ctx.nf_map))
	{
		// create new texture then
		nf_tex_t tn = {0};
		tn.w = 512;
		tn.h = 128;

		// TODO add reusable pool later
		
		const bgfx_memory_t * bitmap_mem = bgfx_alloc(tn.w * tn.h * 4);
		nf_print(bitmap_mem->data, tn.w, tn.h, ctx.nf_font, NULL, 0, &tn.aabb, text);
		tn.tex = bgfx_create_texture_2d(tn.w, tn.h, false, 1, BGFX_TEXTURE_FORMAT_BGRA8, BGFX_TEXTURE_NONE, bitmap_mem);

		int retk = 0;
		k = kh_put_nf_text_map(ctx.nf_map, strdup(text), &retk);
		kh_value(ctx.nf_map, k) = tn;

	}

	nf_tex_t * t = &kh_value(ctx.nf_map, k);
	t->ttl = 3;

	float u1 = (float)t->aabb.x / (float)t->w;
	float v1 = (float)t->aabb.y / (float)t->h;
	float u2 = (float)(t->aabb.x + t->aabb.w) / (float)t->w;
	float v2 = (float)(t->aabb.y + t->aabb.h) / (float)t->h;

	vrtx_t sprite_vertices[4] =
	{
		// 0 1
		// 3 2
		{-0.5f * t->aabb.w + t->aabb.x,  0.5f * t->aabb.h + t->aabb.y, 0.0f, u1, v1, 0xffffffff },
		{ 0.5f * t->aabb.w + t->aabb.x,  0.5f * t->aabb.h + t->aabb.y, 0.0f, u2, v1, 0xffffffff },
		{ 0.5f * t->aabb.w + t->aabb.x, -0.5f * t->aabb.h + t->aabb.y, 0.0f, u2, v2, 0xffffffff },
		{-0.5f * t->aabb.w + t->aabb.x, -0.5f * t->aabb.h + t->aabb.y, 0.0f, u1, v2, 0xffffffff },
	};

	if(out_bounds)
	{
		out_bounds[0] = sprite_vertices[0].x;
		out_bounds[1] = sprite_vertices[2].y;
		out_bounds[2] = sprite_vertices[2].x;
		out_bounds[3] = sprite_vertices[0].y;
	}

	const uint16_t sprite_indices[6] =
	{
		0, 1, 2, // TODO resolve dat shit with ccw vs cw
		0, 2, 3,
	};

	bgfx_transient_vertex_buffer_t vt;
	bgfx_alloc_transient_vertex_buffer(&vt, 4, r_decl());
	memcpy(vt.data, sprite_vertices, sizeof(sprite_vertices));

	bgfx_transient_index_buffer_t it;
	bgfx_alloc_transient_index_buffer(&it, 6);
	memcpy(it.data, sprite_indices, sizeof(sprite_indices));

	// TODO support align

	r_submit_transient(&vt, &it, t->tex, r, g, b, a, BGFX_STATE_DEFAULT_2D | BGFX_STATE_BLEND_ALPHA);
}
#endif

void r_text_ex2(font_t font,
				float x, float y, float deg, float rox, float roy, float sx, float sy, float sox, float soy, float r, float g, float b, float a,
				float * out_bounds, uint8_t align, float size_in_pt, float spacing_in_pt,
				const char * text)
{
	tr_set_world(tr_model_spr(x, y, deg, rox, roy, sx, sy, sox, soy, 1, 1, 0, 0));

	#ifdef NF

	if(font == NATIVE_FONT)
	{
		_r_nf(r, g, b, a, out_bounds, align, text);
		return;
	}

	#endif

	// TODO reenable this when we will need va_args
	//char buffer[16 * 1024];
	//va_list args;
	//va_start(args, text);
	//vsnprintf(buffer, sizeof(buffer), text, args);
	//va_end(args);

	tr_set_world(tr_model_spr(x, y, deg, rox, roy, sx, sy, sox, soy, 1, 1, 0, 0));

	fonsSetFont(ctx.fons, font);
	fonsSetSize(ctx.fons, size_in_pt);
	fonsSetSpacing(ctx.fons, spacing_in_pt);
	fonsSetColor(ctx.fons, r_color(r, g, b, a));
	fonsSetAlign(ctx.fons, align);
	if(out_bounds)
	{
		fonsTextBounds(ctx.fons, 0.0f, 0.0f, text, NULL, out_bounds);
		// TODO do we need to transform them?
	}
	fonsDrawText(ctx.fons, 0.0f, 0.0f, text, NULL);
}

void _r_text_debug_atlas(float k_size, bool blend)
{
	tr_set_world(tr_model_spr(0, 0, 0, 0, 0, 1, 1, 0, 0, ctx.tex_w * k_size, ctx.tex_h * k_size, 0, 0));
	r_submit(_r_sprvbuf(), _r_spribuf(), ctx.tex, 1, 1, 1, 1, BGFX_STATE_DEFAULT_2D | (blend ? BGFX_STATE_BLEND_ALPHA : 0));
}
