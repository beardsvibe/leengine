#pragma once

#include <chipmunk/chipmunk.h>

// default physics configuration:
// - 1 chipmunk2d unit = 1 cm
// - scale is 1 cm = 1 px = 1 chipmunk2d unit
// - gravity is 9.8 m/s^2

void _p_init();
void _p_deinit();
void _p_update(double dt);
void _p_debug_render();

cpSpace * p_space();

void  p_set_scale(float scale); // default 1 cm = 1 px
float p_scale();

void p_remove_body(cpBody * body);
