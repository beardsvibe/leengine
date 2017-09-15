#pragma once

#include "scene.h"
#include "window.h"

// called internally
void _buttons_at_frame_start();

bool is_mouse_inside_any_button();
bool button_update(scene_t * scene, scene_button_t * btn, size_t index, fingers_t fingers, bool is_active);
