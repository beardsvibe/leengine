#pragma once

#include "render.h"

void rb_init();
void rb_deinit();

void rb_start();
void rb_add(bgfx_texture_handle_t texture, const vrtx_t * vbuf, uint16_t vbuf_count, const uint16_t * ibuf, uint32_t ibuf_count, uint64_t state);
void rb_flush();
