
#include "scrollview.h"
#include <math.h>

#define VIEW_DECELERATION_RATE                     1400.0f        // in points/s^2
#define CRITICAL_DAMPED_SPRING_OMEGA               2.0f           // based on UIScrollView
#define CRITICAL_DAMPED_SPRING_OMEGA_OUT_OF_BOUNDS 4.0f * 3.1415f // based on Box2D source
#define RUBBER_BAND_CONSTANT                       0.55f          // based on UIScrollView

// critically damped spring numerical model
// http://mathproofs.blogspot.se/2013/07/critically-damped-spring-smoothing.html
static float critically_damped_spring(float x0, float G, float * v, float omega, float dt)
{
	*v = (*v - (x0 - G) * (omega * omega * dt)) / ((1 + omega * dt) * (1 + omega * dt));
	return x0 + *v * dt;
}

// rubber band model
// x1 = (x0 * d * c) / (d + c * x0)
// x0 – distance from the edge
// c – constant
// d – dimension, either width or height
static float rubber_band(float position, float min, float max, float dimension)
{
	if(position >= min && position <= max)
		return position;
	bool is_under_min = position < min ? true : false;
	float distance_from_edge = is_under_min ? min - position : position - max;
	float rubber_band_pos = (distance_from_edge * dimension * RUBBER_BAND_CONSTANT) / (dimension + RUBBER_BAND_CONSTANT * distance_from_edge);
	return is_under_min ? min - rubber_band_pos : max + rubber_band_pos;
}

// return max bound adjusted to dimension
static float view_pos_max_adj(scrollview_t * sv)
{
	return sv->view_pos_max - (sv->scroll_direction ? sv->view_rect_h : sv->view_rect_w);
}

static void touch_start(scrollview_t * sv, float touch_pos)
{
	// reset frame counter
	sv->drag_frames_count = 0;

	// memorize current touch and view positions
	sv->drag_start_touch_pos = touch_pos;
	sv->drag_start_view_pos = sv->view_current_pos;
}

static void touch_move(scrollview_t * sv, float dt, float touch_pos)
{
	// calculate new position, by stricly following the touch
	sv->view_current_pos = touch_pos - sv->drag_start_touch_pos + sv->drag_start_view_pos;

	// apply rubber band for dragging
	sv->view_current_pos = rubber_band(sv->view_current_pos, sv->view_pos_min, view_pos_max_adj(sv), sv->scroll_direction ? sv->view_rect_h : sv->view_rect_w);

	// push current pos to array
	for(uint8_t i = SCROLLVIEW_DRAG_POS_FRAMES_MAX - 1; i > 0; --i)
		sv->drag_frames[i] = sv->drag_frames[i - 1];
	sv->drag_frames[0].pos = sv->view_current_pos;
	sv->drag_frames[0].dt = dt;
	if(sv->drag_frames_count < SCROLLVIEW_DRAG_POS_FRAMES_MAX)
		++sv->drag_frames_count;
}

static float calculate_average_final_speed(const scrollview_t * sv)
{
	// calculate average final speed on touch end
	// for this we need to calculate average with coefficients
	// samples before roll_off are equaly important (k = 1), and we calculate their average
	// samples after roll_off contribute in decreasing importance k = 1/2, 1/3, 1/4, etc
	// this should provide fast but stable response

	const uint8_t roll_off = 4;

	float spd_total = 0.0f;
	float koef_sum = 0.0f;

	for(uint8_t i = 1; i < sv->drag_frames_count; ++i)
	{
		float spd = (sv->drag_frames[i - 1].pos - sv->drag_frames[i].pos) / sv->drag_frames[i - 1].dt;
		float koef = (i > roll_off) ? (1.0f / (float)(i - roll_off + 1)) : 1.0f;
		spd_total += spd * koef;
		koef_sum += koef;
	}

	return spd_total / koef_sum;
}

static void touch_end(scrollview_t * sv)
{
	// first we calculate where we need to stop
	// we do this by using equation of
	// linear uniformly accelerated motion
	// dist = (stop_speed^2 - start_speed^2) / (2 * acc)
	// http://www.efm.leeds.ac.uk/CIVE/CIVE1140/section01/linear_motion.html
	float start_speed = calculate_average_final_speed(sv);
	float stop_speed = 0.0f;
	float direction = start_speed >= 0.0f ? 1.0f : -1.0f;
	float traveled_dist = fabsf((stop_speed * stop_speed - start_speed * start_speed) / (2.0f * VIEW_DECELERATION_RATE));
	sv->view_target_pos = sv->drag_frames[0].pos + traveled_dist * direction;

	// snap with internal works by snapping taget position to a grid of possible values
	if(sv->snap_with_interval > 0.0f)
	{
		float k = sv->snap_with_interval;
		sv->view_target_pos = k * (int)((sv->view_target_pos + k / 2.0f) / k);
	}

	// set initial velocity of critical damped spring model
	// double sure that it's less then maximal velocity
	// otherwise oscillator will behave as underdamped! (overjump target position)
	// https://web.njit.edu/~kenahn/11spring/phys106/Lecture/L23.pdf
	float max_velocity = CRITICAL_DAMPED_SPRING_OMEGA * traveled_dist * direction;
	sv->view_velocity = fabsf(start_speed) > fabsf(max_velocity) ? max_velocity : start_speed;

	// by default we assume in bounds animation
	sv->view_anim_out_of_bounds = false;
}

static void animate(scrollview_t * sv, float dt)
{
	// as soon as we get out of bounds, force different case even if we go back to bounds
	if(sv->view_current_pos < sv->view_pos_min || sv->view_current_pos > view_pos_max_adj(sv))
		sv->view_anim_out_of_bounds = true;

	if(sv->view_anim_out_of_bounds)
	{
		// hard clamp target position to be in bounds
		if(sv->view_target_pos < sv->view_pos_min)
			sv->view_target_pos = sv->view_pos_min;
		else if(sv->view_target_pos > view_pos_max_adj(sv))
			sv->view_target_pos = view_pos_max_adj(sv);

		// use much faster spring
		sv->view_current_pos = critically_damped_spring(sv->view_current_pos, sv->view_target_pos, &sv->view_velocity, CRITICAL_DAMPED_SPRING_OMEGA_OUT_OF_BOUNDS, dt);
	}
	else
	{
		// normal scrolling
		sv->view_current_pos = critically_damped_spring(sv->view_current_pos, sv->view_target_pos, &sv->view_velocity, CRITICAL_DAMPED_SPRING_OMEGA, dt);
	}
}

void scrollview_update(scrollview_t * sv, float dt, float touch_pos_x, float touch_pos_y, bool touch_press)
{
	if(touch_press && !sv->drag_is_not_in_view)
	{
		// on first touch
		if(!sv->was_dragging)
		{
			// if we are out of bounds
			if(
				touch_pos_x < sv->view_rect_x ||
				touch_pos_y < sv->view_rect_y ||
				touch_pos_x > sv->view_rect_x + sv->view_rect_w ||
				touch_pos_y > sv->view_rect_y + sv->view_rect_h)
			{
				// disable everything until touch is unpressed
				sv->drag_is_not_in_view = true;
				return;
			}
			else
			{
				// otherwise start dragging
				sv->was_dragging = true;
				touch_start(sv, sv->scroll_direction ? touch_pos_y : touch_pos_x);
			}
		}

		// update dragging
		touch_move(sv, dt, sv->scroll_direction ? touch_pos_y : touch_pos_x);
	}
	else
	{
		// on last touch
		if(!touch_press)
		{
			// if we had a disabled touch - reenable it
			sv->drag_is_not_in_view = false;

			// if we were dragging - end it
			if(sv->was_dragging)
			{
				sv->was_dragging = false;
				touch_end(sv);
			}
		}

		// do normal animation update
		animate(sv, dt);
	}
}
