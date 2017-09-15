#include "buttons.h"

static struct
{
	bool is_mouse_inside_any_button;
	bool started_button_press[MAX_FINGERS];
} ctx = {0};

static bool is_in_sprite(scene_t * scene, scene_entity_t * ent, gbVec2 mouse)
{
	if(!ent->visible)
		return false;

	#if 1
	scene_entities_list_t all_sprites = scene_get_entities_for_prefix(scene, ent->name);
	gbRect2 r = sprites_AABB(&all_sprites, false);

	//#ifdef __APPLE__
	gbVec2 mid = gb_vec2(r.pos.x + r.dim.x / 2.0f, r.pos.y + r.dim.y / 2.0f);
	if(r.dim.x < 100.0f)
		r.dim.x = 100.0f;
	if(r.dim.y < 100.0f)
		r.dim.y = 100.0f;
	r.pos.x = mid.x - r.dim.x / 2.0f;
	r.pos.y = mid.y - r.dim.y / 2.0f;
	//#endif

	#else
	gbVec2 rs = gb_vec2(ent->sx * ent->start_w, ent->sy * ent->start_h);

	//#ifdef __APPLE__
	// TODO for buttons
	if(rs.x < 100.0f)
		rs.x = 100.0f;
	if(rs.y < 100.0f)
		rs.y = 100.0f;
	//#endif

	gbRect2 r = gb_rect2(gb_vec2(ent->x - rs.x / 2.0f, ent->y - rs.y / 2.0f), rs);
	#endif
	return gb_rect2_contains_vec2(r, mouse) ? true : false;
}

void _buttons_at_frame_start()
{
	ctx.is_mouse_inside_any_button = false;
}

bool is_mouse_inside_any_button()
{
	return ctx.is_mouse_inside_any_button;
}

bool button_update(scene_t * scene, scene_button_t * btn, size_t index, fingers_t fingers, bool is_active)
{
	bool result = false;

	if(fingers.hit[0])
	{
		bool was = btn->mouse_in;
		bool now = is_in_sprite(scene, btn->enabled, fingers.pos[0]);

		if(!was && now)
		{
			btn->mouse_in = true;
			btn->is_pressed = true;
			btn->btn_index = index;
			//ep_log("pressed on %s:%u\n", btn->name, (uint32_t)index);
		}
	}
	else if(fingers.touch[0])
	{
		if(btn->is_pressed && btn->btn_index == index)
		{
			btn->mouse_in = is_in_sprite(scene, btn->enabled, fingers.pos[0]);
		}
	}
	else if(btn->is_pressed && btn->btn_index == index)
	{
		if(btn->mouse_in && btn->is_pressed)
		{
			// hit
			result = true;
		}
		btn->mouse_in = false;
		btn->is_pressed = false;
		btn->btn_index = -1;
	}

	if(btn->enabled)
		btn->enabled->visible &= !btn->is_pressed && (!is_active || !btn->activated);
	if(btn->disabled)
		btn->disabled->visible = false;
	if(btn->touched)
		btn->touched->visible &= btn->is_pressed;
	if(btn->activated)
		btn->activated->visible &= !btn->is_pressed && is_active;

	return result;
}
