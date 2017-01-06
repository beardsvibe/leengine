#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <entrypoint.h>

// -----------------------------------------------------------------------------
// implement this in your codebase
int32_t game_init(int32_t argc, char * argv[]); // 0 is ok
int32_t game_deinit(); // 0 is ok, don't really rely on this to be called
int32_t game_might_unload(); // 0 is ok
int32_t game_update(uint16_t w, uint16_t h, float dt); // 0 is ok, != 0 means we want to exit if possible
int32_t game_render(uint16_t w, uint16_t h, float dt); // 0 is ok, != 0 means we want to exit if possible

// -----------------------------------------------------------------------------

uint16_t w_width();
uint16_t w_height();

#define DBG_NONE		0x0
#define DBG_TEXT		0x1
#define DBG_WIREFRAME	0x2
#define DBG_STATS		0x4
void w_dbg(uint32_t options);

// mouse
float w_mx();
float w_my();
bool w_mtouch();
bool w_ml();
bool w_mr();

// touch
uint8_t w_tmax();
float w_tx(uint8_t i);
float w_ty(uint8_t i);
bool w_touch(uint8_t i);
