
#include "scene.h"
#include <stdio.h>
#include <string.h>

void scene_free(scene_t * scene)
{
	for(size_t i = 0; i < scene->textures_count; ++i)
		r_free(*scene->textures[i]);
}

void scene_draw(scene_t * scene)
{
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

		r_render_ex2(
			c->tex,
			e->x, e->y,
			e->r, e->rox, e->roy,
			e->sx, e->sy, e->sox, e->soy, e->ox, e->oy,
			c->diffuse.r, c->diffuse.g, c->diffuse.b, c->diffuse.a
		);
	}

	if(e->text)
	{
		scene_text_t * c = e->text;

		if(c->shadow)
		{
			r_text_ex2(
				c->font,
				e->x + c->shadow_x, e->y + c->shadow_y,
				e->r, e->rox, e->roy,
				e->sx, e->sy, e->sox, e->soy,
				c->shadow_diffuse.r, c->shadow_diffuse.g, c->shadow_diffuse.b, c->shadow_diffuse.a,
				NULL,
				TEXT_ALIGN_CENTER | TEXT_ALIGN_MIDDLE,
				e->start_h,
				0.0f,
				c->text
			);
		}

		r_text_ex2(
			c->font,
			e->x, e->y,
			e->r, e->rox, e->roy,
			e->sx, e->sy, e->sox, e->soy,
			c->diffuse.r, c->diffuse.g, c->diffuse.b, c->diffuse.a,
			NULL,
			TEXT_ALIGN_CENTER | TEXT_ALIGN_MIDDLE,
			e->start_h,
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
