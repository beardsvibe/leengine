
#include "text.h"
#include "render.h"
#include "filesystem.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#define FONTSTASH_IMPLEMENTATION
#include <fontstash.h>

static struct
{
	FONScontext * fons;
	bool tex_valid;
	bgfx_texture_handle_t tex;
	uint32_t tex_w, tex_h;
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
			((r_color_t*)mem->data)[j * w + i] = r_to_color(255, 255, 255, c);
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
		id[i * 3 + 0] = i * 3 + 0;
		id[i * 3 + 1] = i * 3 + 2;
		id[i * 3 + 2] = i * 3 + 1;
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
}

void _t_deinit()
{
	fonsDeleteInternal(ctx.fons);
}

int32_t t_add(const char * fontname, const char * filename)
{
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

	return fonsAddFontMem(ctx.fons, fontname, data, size, 1);
}

void r_text_ex2(int32_t font,
				float x, float y, float deg, float sx, float sy, float ox, float oy, float r, float g, float b, float a,
				float * out_bounds, uint8_t align, float size_in_pt, float spacing_in_pt,
				const char * text, ...)
{
	char buffer[16 * 1024];
	va_list args;
	va_start(args, text);
	vsnprintf(buffer, sizeof(buffer), text, args);
	va_end(args);

	tr_set_world(tr_model_spr(x, y, deg, sx, sy, ox, oy, 1, 1));

	fonsSetFont(ctx.fons, font);
	fonsSetSize(ctx.fons, size_in_pt);
	fonsSetSpacing(ctx.fons, spacing_in_pt);
	fonsSetColor(ctx.fons, r_to_colorf(r, g, b, a));
	fonsSetAlign(ctx.fons, align);
	if(out_bounds)
	{
		fonsTextBounds(ctx.fons, 0.0f, 0.0f, buffer, NULL, out_bounds);
		// TODO do we need to transform them?
	}
	fonsDrawText(ctx.fons, 0.0f, 0.0f, buffer, NULL);
}

void _r_text_debug_atlas(float k_size, bool blend)
{
	tr_set_world(tr_model_spr(0, 0, 0, 1, 1, 0, 0, ctx.tex_w * k_size, ctx.tex_h * k_size));
	r_submit(_r_sprvbuf(), _r_spribuf(), ctx.tex, 1, 1, 1, 1, BGFX_STATE_DEFAULT_2D | (blend ? BGFX_STATE_BLEND_ALPHA : 0));
}
