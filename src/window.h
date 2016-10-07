#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef void (*w_initlike_t)(void * ctx);
typedef void (*w_loop_t)(uint16_t w, uint16_t h, float dt, void * ctx);

// call this first of all
int w_run_loop(w_initlike_t init_func, w_initlike_t deinit_func, w_loop_t update_func, w_loop_t render_func, void * ctx);
void w_stop(); // call this to stop run loop

bool w_init(const char * name, uint16_t w, uint16_t h); // w_init will init other stuff (like render, sound, etc) for you
void w_deinit();

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

// keys
typedef enum { // borrowed from tigr
	K_PAD0=128,K_PAD1,K_PAD2,K_PAD3,K_PAD4,K_PAD5,K_PAD6,K_PAD7,K_PAD8,K_PAD9,
	K_PADMUL,K_PADADD,K_PADENTER,K_PADSUB,K_PADDOT,K_PADDIV,
	K_F1,K_F2,K_F3,K_F4,K_F5,K_F6,K_F7,K_F8,K_F9,K_F10,K_F11,K_F12,
	K_BACKSPACE,K_TAB,K_RETURN,K_PAUSE,K_CAPSLOCK,
	K_ESCAPE,K_SPACE,K_PAGEUP,K_PAGEDN,K_END,K_HOME,K_LEFT,K_UP,K_RIGHT,K_DOWN,
	K_INSERT,K_DELETE,K_LWIN,K_RWIN,K_NUMLOCK,K_SCROLL,K_LSHIFT,K_RSHIFT,
	K_LCONTROL,K_RCONTROL,K_LALT,K_RALT,K_SEMICOLON,K_EQUALS,K_COMMA,K_MINUS,
	K_DOT,K_SLASH,K_BACKTICK,K_LSQUARE,K_BACKSLASH,K_RSQUARE,K_TICK
} w_key_t;

bool w_k_down(uint8_t key); // use ascii codes a-z, 0-9
bool w_k_hit(uint8_t key); // will return true if specified key was hit in current frame
