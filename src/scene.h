#pragma once

// this is "interface" to scenes
// actual implementation is generated with psd slice tool (or from code)

#include "render.h"
#include "render_text.h"
#include "render_9slice.h"

typedef font_t (*scene_load_font_t)(const char * name); // TODO temporarely solution

typedef struct scene_entity_t scene_entity_t;
typedef struct scene_sprite_t scene_sprite_t;
typedef struct scene_text_t scene_text_t;
typedef struct scene_button_t scene_button_t;
typedef struct scene_t scene_t;

// -----------------------------------------------------------------------------

typedef void (*scene_callback_t)(scene_entity_t * entity, scene_t * scene);

struct scene_entity_t
{
	float x, y, r, rox, roy, sx, sy, sox, soy, ox, oy;	// position stuff

	float start_x, start_y;								// original position of an object
	float start_w, start_h;								// original size

	bool visible; // TODO rename to enabled

	const char * name; // object name, same as in psd

	scene_callback_t callback;
	uintptr_t userdata;

	// components,
	scene_sprite_t * sprite;
	scene_text_t * text;
};

// -----------------------------------------------------------------------------

struct scene_sprite_t
{
	scene_entity_t * entity;

	r_colorf_t diffuse;

	tex_t tex;

	// if present, render current sprite as 9slice
	tex_9slice_t * tex_9slice;

	bool pixel_perfect;
};

struct scene_text_t
{
	scene_entity_t * entity;

	const char * text;
	const char * original_text;

	float size_in_pt;
	font_t font;
	r_colorf_t diffuse;
	bool use_native_font;

	bool shadow;
	float shadow_x, shadow_y;
	r_colorf_t shadow_diffuse;

	bool new_style;
	float size_in_px;
	uint8_t align;
	gbRect2 bounds; // bounds feedback after rendering
};

struct scene_button_t
{
	char name[256];
	scene_entity_t * enabled;
	scene_entity_t * disabled;
	scene_entity_t * touched;
	scene_entity_t * activated;
	bool mouse_in;
	bool is_pressed;
	size_t btn_index;
};

// -----------------------------------------------------------------------------

#define SCENE_PASS_DRAW 1
#define SCENE_PASS_FREE 2
typedef void (*scene_pass_callback_t)(scene_t * scene, uint8_t at_pass);

struct scene_t
{
	scene_entity_t ** entities; // sorted in rendering order
	size_t entities_count;

	tex_t ** textures;
	size_t textures_count;

	tex_9slice_t ** tex_9slices;
	size_t tex_9slices_count;

	scene_sprite_t ** sprites;
	size_t sprites_count;

	scene_text_t ** texts;
	size_t texts_count;

	scene_pass_callback_t pass_callback;
	uintptr_t userdata;
};

void scene_free(scene_t * scene);
void scene_draw(scene_t * scene);
void scene_draw_entity(scene_entity_t * entity);

// -----------------------------------------------------------------------------

#define SCENE_MAX_ENT_SEARCH_COUNT 128
typedef struct
{
	scene_entity_t * entities[SCENE_MAX_ENT_SEARCH_COUNT];
	size_t count;
} scene_entities_list_t;
scene_entities_list_t scene_get_entities_for_prefix(scene_t * scene, const char * prefix);
void scene_set_entities_visibility(scene_entities_list_t * entities, bool visible);
void scene_set_entities_visibility_for_prefix(scene_t * scene, const char * prefix, bool visible);

// AABB in parent space
gbRect2 sprite_AABB(scene_sprite_t * sprite, bool original);
gbRect2 sprites_AABB(scene_entities_list_t * entities, bool original);
