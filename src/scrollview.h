
// 1D kinetic scrolling
// similar by feel to iOS UIScrollView

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SCROLLVIEW_DRAG_POS_FRAMES_MAX 20

typedef struct
{
	// ----------------------------- params, set outside
	float view_rect_x;
	float view_rect_y;
	float view_rect_w;
	float view_rect_h;

	float view_pos_min;
	float view_pos_max;

	bool scroll_direction; // true - vertical

	float snap_with_interval; // if > 0, will snap to specific positions

	// ----------------------------- current position
	float view_current_pos;

	// ----------------------------- internal parameters

	// animation
	float view_velocity;
	float view_target_pos;
	bool view_anim_out_of_bounds;

	// dragging
	float drag_start_view_pos;
	float drag_start_touch_pos;
	struct
	{
		float pos;
		float dt;
	} drag_frames[SCROLLVIEW_DRAG_POS_FRAMES_MAX];
	uint8_t drag_frames_count;
	bool was_dragging;
	bool drag_is_not_in_view;
} scrollview_t;

void scrollview_update(scrollview_t * sv, float dt, float touch_pos_x, float touch_pos_y, bool touch_press);
