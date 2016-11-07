#include "physics_debug.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#define MAX_LINES 16 * 1024

typedef struct
{
	float x1, y1, x2, y2;
	r_color_t color;
} line_t;

static struct
{
	line_t lines[MAX_LINES];
	size_t lines_count;
} ctx;

static void _line(cpVect a, cpVect b, cpSpaceDebugColor c)
{
	if(ctx.lines_count >= MAX_LINES)
		return;
	line_t * l = &ctx.lines[ctx.lines_count];
	l->x1 = a.x; l->y1 = a.y; l->x2 = b.x; l->y2 = b.y;
	l->color = r_to_color(c.r * 255.0f, c.g * 255.0f, c.b * 255.0f, c.a * 255.0f);
	ctx.lines_count++;
	if(ctx.lines_count >= MAX_LINES)
		printf("reached max lines in physics debug renderer!\n");
}

static void _r_circle(cpVect pos, cpFloat angle, cpFloat radius, cpSpaceDebugColor outline_color, cpSpaceDebugColor fill_color, cpDataPointer data)
{
	float lx = radius + pos.x, ly = pos.y;
	float steps = 10.0f;
	float k = 2.0f * M_PI / steps;
	for(float a = 0.0f; a <= 2.0f * M_PI; a += k)
	{
		float x = cosf(a + k) * radius + pos.x, y = sinf(a + k) * radius + pos.y;
		_line(cpv(x, y), cpv(lx, ly), fill_color);
		lx = x; ly = y;
	}
}

static void _r_segment(cpVect a, cpVect b, cpSpaceDebugColor color, cpDataPointer data)
{
	//_line(a, b, color);
}

static void _r_fat_segment(cpVect a, cpVect b, cpFloat radius, cpSpaceDebugColor outline_color, cpSpaceDebugColor fill_color, cpDataPointer data)
{
	_line(a, b, fill_color);
}

static void _r_polygon(int count, const cpVect *verts, cpFloat radius, cpSpaceDebugColor outline_color, cpSpaceDebugColor fill_color, cpDataPointer data)
{
	for(int i = 0; i < count; ++i)
		_line(verts[i ? i - 1 : count - 1], verts[i], fill_color);
}

static void _r_dot(cpFloat size, cpVect pos, cpSpaceDebugColor color, cpDataPointer data)
{
	_line(pos, pos, color);
}

static cpSpaceDebugColor _r_shape_color(cpShape * shape, cpDataPointer data)
{
	cpSpaceDebugColor wut = {1.0f, 1.0f, 1.0f, 1.0f};
	return wut;
}

static cpSpaceDebugDrawOptions opt =
{
	_r_circle,
	_r_segment,
	_r_fat_segment,
	_r_polygon,
	_r_dot,
	CP_SPACE_DEBUG_DRAW_SHAPES | CP_SPACE_DEBUG_DRAW_CONSTRAINTS | CP_SPACE_DEBUG_DRAW_COLLISION_POINTS,
	{1.0f, 1.0f, 1.0f, 1.0f}, // shape outline color
	_r_shape_color,
	{0.0f, 1.0f, 0.0f, 1.0f}, // constraint color
	{1.0f, 0.0f, 0.0f, 1.0f}, // collision point color
	NULL // data pointer
};

cpSpaceDebugDrawOptions * p_debug_opt()
{
	return &opt;
}

void p_debug_flush(float scale)
{
	bgfx_transient_vertex_buffer_t vt;
	bgfx_alloc_transient_vertex_buffer(&vt, ctx.lines_count * 2, r_decl());
	vrtx_t * vert = (vrtx_t*)vt.data;

	for(size_t i = 0; i < ctx.lines_count; ++i)
	{
		line_t * l = &ctx.lines[i];
		vert[i * 2 + 0].x = l->x1 * scale;
		vert[i * 2 + 0].y = l->y1 * scale;
		vert[i * 2 + 0].z = 0.0f;
		vert[i * 2 + 0].u = 0.0f;
		vert[i * 2 + 0].v = 0.0f;
		vert[i * 2 + 0].color = l->color;
		vert[i * 2 + 1].x = l->x2 * scale;
		vert[i * 2 + 1].y = l->y2 * scale;
		vert[i * 2 + 1].z = 0.0f;
		vert[i * 2 + 1].u = 0.0f;
		vert[i * 2 + 1].v = 0.0f;
		vert[i * 2 + 1].color = l->color;
	}

	bgfx_transient_index_buffer_t it;
	bgfx_alloc_transient_index_buffer(&it, ctx.lines_count * 2);
	uint16_t * id = (uint16_t*)it.data;

	for(size_t i = 0; i < ctx.lines_count; ++i)
	{
		id[i * 2 + 0] = i * 2 + 0;
		id[i * 2 + 1] = i * 2 + 1;
	}

	tr_set_world(tr_identity());
	r_submit_transient(&vt, &it, r_white_tex().tex, 1.0f, 1.0f, 1.0f, 1.0f, BGFX_STATE_DEFAULT_2D | BGFX_STATE_PT_LINES);

	ctx.lines_count = 0;
}
