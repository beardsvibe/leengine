
#include "scene.h"
#include <stdio.h>
#include <string.h>

void scene_free(scene_t * scene)
{
	if(scene->pass_callback)
		scene->pass_callback(scene, SCENE_PASS_FREE);

	for(size_t i = 0; i < scene->textures_count; ++i)
		r_free(*scene->textures[i]);
}

void scene_draw(scene_t * scene)
{
	if(scene->pass_callback)
		scene->pass_callback(scene, SCENE_PASS_DRAW);

	for(size_t i = 0; i < scene->entities_count; ++i)
	{
		scene_entity_t * e = scene->entities[i];

		if(e->callback)
			e->callback(e, scene);

		if(e->visible)
			scene_draw_entity(e);
	}
}

void scene_draw_entity(scene_entity_t * e)
{
	if(e->sprite)
	{
		scene_sprite_t * c = e->sprite;

		if(c->tex_9slice)
		{
			r_9slice(
				c->tex, *c->tex_9slice,
				e->start_w * e->sx, e->start_h * e->sy,
				e->x, e->y,
				e->r, e->rox, e->roy,
				e->ox, e->oy,
				c->diffuse.r, c->diffuse.g, c->diffuse.b, c->diffuse.a,
				c->pixel_perfect
			);
		}
		else
		{
			r_render_sprite_ex(
				c->tex,
				e->x, e->y,
				e->r, e->rox, e->roy,
				e->sx, e->sy, e->sox, e->soy, e->ox, e->oy,
				c->diffuse.r, c->diffuse.g, c->diffuse.b, c->diffuse.a,
				c->pixel_perfect
			);
		}
	}

	if(e->text)
	{
		scene_text_t * c = e->text;
		r_text_ex2(
			c->use_native_font ? NATIVE_FONT : c->font,
			e->x, e->y,
			e->r, e->rox, e->roy,
			e->sx, e->sy, e->sox, e->soy,
			c->diffuse.r, c->diffuse.g, c->diffuse.b, c->diffuse.a,
			c->shadow,
			c->shadow_x, c->shadow_y,
			c->shadow_diffuse.r, c->shadow_diffuse.g, c->shadow_diffuse.b, c->shadow_diffuse.a,
			c->new_style ? &c->bounds : NULL,
			c->new_style ? c->align : TEXT_ALIGN_CENTER | TEXT_ALIGN_MIDDLE,
			c->new_style ? c->size_in_px : e->start_h,
			0.0f,
			c->text
		);
	}
}

scene_entities_list_t scene_get_entities_for_prefix(scene_t * scene, const char * prefix)
{
	scene_entities_list_t r = {0};
	for(size_t i = 0; i < scene->entities_count; ++i)
	{
		scene_entity_t * e = scene->entities[i];
		if((strlen(e->name) >= strlen(prefix)) && (!strncmp(prefix, e->name, strlen(prefix))))
		{
			if(r.count < sizeof(r.entities) / sizeof(r.entities[0]))
				r.entities[r.count++] = e;
			else
				printf("no more space in entities list for prefix %s\n", prefix);
		}
	}
	return r;
}

void scene_set_entities_visibility(scene_entities_list_t * entities, bool visible)
{
	if(!entities)
		return;

	for(size_t i = 0; i < entities->count; ++i)
		entities->entities[i]->visible = visible;
}

void scene_set_entities_visibility_for_prefix(scene_t * scene, const char * prefix, bool visible)
{
	scene_entities_list_t ent = scene_get_entities_for_prefix(scene, prefix);
	scene_set_entities_visibility(&ent, visible);
}

gbRect2 sprite_AABB(scene_sprite_t * c, bool original)
{
	if(!c)
	{
		gbRect2 r = {0};
		return r;
	}

	scene_entity_t * e = c->entity;

	if(original)
	{
		return gb_rect2(
			gb_vec2(e->start_x - e->start_w / 2.0f, e->start_y - e->start_h / 2.0f),
			gb_vec2(e->start_w, e->start_h)
		);
	}

	trns_t model = tr_model_spr(
		e->x, e->y,
		e->r, e->rox, e->roy,
		e->sx, e->sy, e->sox, e->soy,
		c->tex.w, c->tex.h,
		e->ox, e->oy
	);

	const gbVec4 sprite_vertices[4] =
	{
		{-0.5f,  0.5f, 0.0f, 1.0f},
		{ 0.5f,  0.5f, 0.0f, 1.0f},
		{ 0.5f, -0.5f, 0.0f, 1.0f},
		{-0.5f, -0.5f, 0.0f, 1.0f},
	};

	gbVec4 results[4] = {0};

	for(uint8_t i = 0; i < 4; ++i)
		gb_mat4_mul_vec4(&results[i], &model, sprite_vertices[i]);

	gbRect2 ret = {0};
	for(uint8_t i = 0; i < 4; ++i)
	{
		gbRect2 cur = gb_rect2(results[i].xy, gb_vec2_zero());
		ret = i ? gb_rect2_union(ret, cur) : cur;
	}

	return ret;
}

gbRect2 sprites_AABB(scene_entities_list_t * entities, bool original)
{
	gbRect2 r = {0};

	if(!entities)
		return r;

	bool first = true;

	for(size_t i = 0; i < entities->count; ++i)
	{
		if(entities->entities[i]->sprite)
		{
			gbRect2 spr = sprite_AABB(entities->entities[i]->sprite, original);
			if(first)
			{
				r = spr;
				first = false;
			}
			else
			{
				r = gb_rect2_union(r, spr);
			}
		}
	}
	return r;
}
