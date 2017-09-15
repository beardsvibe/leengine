
#include "render_batch.h"
#include <stdlib.h>
#include <memory.h>

#include <entrypoint.h>

#ifndef NO_BATCHING

#define MAX_CMD_COUNT (4 * 1024)
#define MAX_V_COUNT (64 * 1024)
#define MAX_I_COUNT (64 * 1024)
#define MAX_BUF_COUNT (8)

typedef struct
{
	bgfx_texture_handle_t tex;
	uint32_t v;
	uint32_t i;
	uint32_t vc;
	uint32_t ic;
	uint64_t state;
} batch_cmd_t;

typedef struct
{
	vrtx_t v[MAX_V_COUNT];
	uint16_t i[MAX_I_COUNT];
	uint32_t v_count;
	uint32_t i_count;
	bgfx_dynamic_vertex_buffer_handle_t vbuf;
	bgfx_dynamic_index_buffer_handle_t ibuf;
} batch_mem_t;

typedef struct
{
	uint32_t dip;
	uint32_t force_flush;
	uint32_t not_enough_buffers;
} batch_stats_t;

static struct
{
	batch_cmd_t cmds[MAX_CMD_COUNT];
	size_t cmds_count;

	struct
	{
		batch_mem_t mem[MAX_BUF_COUNT];
	} frames[2];

	uint8_t current_buffer;
	uint8_t current_frame;

	batch_stats_t stats;

} ctx = {0};

void rb_init()
{
	for(size_t i = 0; i < 2; ++i)
		for(size_t j = 0; j < MAX_BUF_COUNT; ++j)
		{
			batch_mem_t * mem = ctx.frames[i].mem + j;
			mem->vbuf = bgfx_create_dynamic_vertex_buffer(MAX_V_COUNT, r_decl(), BGFX_BUFFER_NONE);
			mem->ibuf = bgfx_create_dynamic_index_buffer(MAX_I_COUNT, BGFX_BUFFER_NONE);
		}
}

void rb_deinit()
{
	for(size_t i = 0; i < 2; ++i)
		for(size_t j = 0; j < MAX_BUF_COUNT; ++j)
		{
			batch_mem_t * mem = ctx.frames[i].mem + j;
			bgfx_destroy_dynamic_vertex_buffer(mem->vbuf);
			bgfx_destroy_dynamic_index_buffer(mem->ibuf);
		}
}

void rb_start()
{
//	ep_log("frame stats: %u %u %u\n", ctx.stats.dip, ctx.stats.force_flush, ctx.stats.not_enough_buffers);
	memset(&ctx.stats, 0, sizeof(batch_stats_t));

	for(size_t i = 0; i < 2; ++i)
		ctx.current_buffer = 0;
	ctx.current_frame = 1 - ctx.current_frame;
}

void rb_add(bgfx_texture_handle_t tex, const vrtx_t * vbuf, uint16_t vbuf_count, const uint16_t * ibuf, uint32_t ibuf_count, uint64_t state)
{
	batch_mem_t * mem = ctx.frames[ctx.current_frame].mem + ctx.current_buffer;

	if(
		(ctx.cmds_count >= MAX_CMD_COUNT) ||
		(mem->v_count + ((size_t)vbuf_count) > MAX_V_COUNT) ||
		(mem->i_count + ((size_t)ibuf_count) > MAX_I_COUNT)
	)
	{
		ctx.stats.force_flush++;
		rb_flush();
		mem = ctx.frames[ctx.current_frame].mem + ctx.current_buffer;
	}

	// copy indexes with offset
	for(size_t i = 0; i < ibuf_count; ++i)
		mem->i[mem->i_count + i] = ibuf[i] + (uint16_t)mem->v_count;

	// copy vertexes
	memcpy(mem->v + mem->v_count, vbuf, vbuf_count * sizeof(vrtx_t));

	// add command
	batch_cmd_t * c = ctx.cmds + ctx.cmds_count;
	c->tex = tex;
	c->state = state;
	c->v = mem->v_count;
	c->vc = vbuf_count;
	c->i = mem->i_count;
	c->ic = ibuf_count;

	// increase counters
	ctx.cmds_count++;
	mem->v_count += vbuf_count;
	mem->i_count += ibuf_count;
}

void rb_flush()
{
	batch_mem_t * mem = ctx.frames[ctx.current_frame].mem + ctx.current_buffer;

	if(!ctx.cmds_count || !mem->v_count || !mem->i_count)
	{
		ctx.cmds_count = 0;
		mem->v_count = 0;
		mem->i_count = 0;
		return;
	}

	// update buffers for a whole batch
	bgfx_update_dynamic_vertex_buffer(mem->vbuf, 0, bgfx_make_ref(mem->v, (uint32_t)mem->v_count * sizeof(vrtx_t)));
	bgfx_update_dynamic_index_buffer(mem->ibuf, 0, bgfx_make_ref(mem->i, (uint32_t)mem->i_count * sizeof(uint16_t)));

	// exec cmds
	bool batch = false;
	uint32_t batch_i_start = 0;
	uint32_t batch_i_size = 0;

	for(size_t i = 0; i < ctx.cmds_count; ++i)
	{
		batch_cmd_t * c = ctx.cmds + i;
		batch_cmd_t * cn = (i + 1 < ctx.cmds_count) ? ctx.cmds + i + 1 : NULL;
		bool can_batch_with_next = cn && (c->state == cn->state) && (c->tex.idx == cn->tex.idx);

		// if no batch, push current one to it
		if(!batch)
		{
			batch = true;
			batch_i_start = c->i;
			batch_i_size = 0;
		}

		batch_i_size += c->ic;

		if(!can_batch_with_next)
		{
			bgfx_set_dynamic_vertex_buffer(0, mem->vbuf, 0, mem->v_count);
			bgfx_set_dynamic_index_buffer(mem->ibuf, batch_i_start, batch_i_size);
			bgfx_set_texture(0, r_s_texture(), c->tex, -1);
			bgfx_set_state(c->state, 0);
			bgfx_submit(r_viewid(), r_prog(), 0, false);

			ctx.stats.dip++;
			batch = false;
		}
	}

	// clear
	ctx.cmds_count = 0;
	mem->v_count = 0;
	mem->i_count = 0;

	// move to next buffer
	ctx.current_buffer++;

	// cycle back to a first buffer if needed
	if(ctx.current_buffer >= MAX_BUF_COUNT)
	{
		ep_log("%s: NOT ENOUGH BUFFERS", __func__);
		ctx.stats.not_enough_buffers++;
		ctx.current_buffer = 0;
	}
}

#else

void rb_init() {}
void rb_deinit() {}
void rb_start() {}
void rb_flush() {}

void rb_add(bgfx_texture_handle_t texture, const vrtx_t * vbuf, uint16_t vbuf_count, const uint16_t * ibuf, uint32_t ibuf_count, uint64_t state)
{
	bgfx_transient_vertex_buffer_t vb;
	bgfx_transient_index_buffer_t ib;
	bgfx_alloc_transient_buffers(&vb, r_decl(), vbuf_count, &ib, ibuf_count);
	memcpy(vb.data, vbuf, vb.size);
	memcpy(ib.data, ibuf, ib.size);

	bgfx_set_transient_vertex_buffer(0, &vb, 0, vbuf_count);
	bgfx_set_transient_index_buffer(&ib, 0, ibuf_count);
	bgfx_set_texture(0, r_s_texture(), texture, -1);
	bgfx_set_state(state, 0);
	bgfx_submit(r_viewid(), r_prog(), 0, false);
}

#endif
