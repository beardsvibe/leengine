#include "window.h"
#include "render.h"
#include "sound.h"
#include "physics.h"
#include <SDL.h>
#include <SDL_syswm.h>
#include <bgfxplatform.h>
#include <stdio.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

static struct
{
	SDL_Window * window;
	uint16_t w, h;
	uint32_t reset_flags;

	struct
	{
		bool touched;
		float x, y;
		bool l, r;
	} mouse;

	struct
	{
		SDL_KeyboardEvent event;
		bool hit;
	} keys[SDL_NUM_SCANCODES];
} ctx;

static struct
{
	w_initlike_t init;
	w_initlike_t deinit;
	w_loop_t update;
	w_loop_t render;
	void * ctx;
	uint32_t time;
	bool run;
	bool want_stop;
	bool first_tick;
} loop_ctx;

static bool w_poll();

static void w_loop()
{
	float dt = 1.0f / 60.0f;
	if(loop_ctx.first_tick)
	{
		loop_ctx.time = SDL_GetTicks();
		loop_ctx.first_tick = false;
		loop_ctx.init(loop_ctx.ctx);
	}
	else
	{
		uint32_t current = SDL_GetTicks();
		dt = (float)(current - loop_ctx.time) / 1000.0f;
		loop_ctx.time = current;
	}

	if(dt > 1.0f / 25.0f)
		dt = 1.0f / 25.0f;

	loop_ctx.run = w_poll() && (!loop_ctx.want_stop);
	loop_ctx.update(ctx.w, ctx.h, dt, loop_ctx.ctx);
	_s_update();
	_p_update(dt);
	loop_ctx.render(ctx.w, ctx.h, dt, loop_ctx.ctx);
	//_p_debug_render();
	bgfx_frame(false);
}

int w_run_loop(w_initlike_t init_func, w_initlike_t deinit_func, w_loop_t update_func, w_loop_t render_func, void * ctx)
{
	loop_ctx.init = init_func;
	loop_ctx.deinit = deinit_func;
	loop_ctx.update = update_func;
	loop_ctx.render = render_func;
	loop_ctx.ctx = ctx;
	loop_ctx.run = true;
	loop_ctx.want_stop = false;
	loop_ctx.first_tick = true;

	#ifdef EMSCRIPTEN
	emscripten_set_main_loop(&w_loop, -1, 1);
	#else
	while(loop_ctx.run)
		w_loop();
	#endif
	loop_ctx.deinit(loop_ctx.ctx);
	return 0;
}

void w_stop()
{
	loop_ctx.want_stop = true;
}

bool w_init(const char * name, uint16_t w, uint16_t h)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return false;

	ctx.w = w;
	ctx.h = h;
	#ifdef EMSCRIPTEN
	ctx.reset_flags = BGFX_RESET_NONE;
	#else
	ctx.reset_flags = BGFX_RESET_VSYNC;
	#endif

	ctx.window = SDL_CreateWindow(
				name,
				SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				ctx.w, ctx.h,
				SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if(SDL_GetWindowWMInfo(ctx.window, &wmi))
	{
		bgfx_platform_data_t pd = {0};
		#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
		pd.ndt          = wmi.info.x11.display;
		pd.nwh          = (void*)(uintptr_t)wmi.info.x11.window;
		#elif BX_PLATFORM_OSX
		pd.nwh          = wmi.info.cocoa.window;
		#elif BX_PLATFORM_WINDOWS
		pd.nwh          = wmi.info.win.window;
		#elif BX_PLATFORM_STEAMLINK
		pd.ndt          = wmi.info.vivante.display;
		pd.nwh          = wmi.info.vivante.window;
		#endif
		bgfx_set_platform_data(&pd);
	}

	bgfx_init(BGFX_RENDERER_TYPE_COUNT, BGFX_PCI_ID_NONE, 0, NULL, NULL);
	bgfx_reset(ctx.w, ctx.h, ctx.reset_flags);

	_r_init();
	_s_init();
	_p_init();
	return true;
}

void w_deinit()
{
	_p_deinit();
	_s_deinit();
	_r_deinit();

	bgfx_shutdown();
	SDL_DestroyWindow(ctx.window);
	SDL_Quit();
}

static bool w_poll()
{
	for(size_t i = 0; i < SDL_NUM_SCANCODES; ++i)
		ctx.keys[i].hit = false;

	SDL_Event e;
	while(SDL_PollEvent(&e) != 0)
	{
		switch(e.type)
		{
		case SDL_QUIT:
			return false;

		case SDL_KEYDOWN:
		{
			ctx.keys[e.key.keysym.scancode].hit = ctx.keys[e.key.keysym.scancode].event.state != e.key.state;
			ctx.keys[e.key.keysym.scancode].event = e.key;
			break;
		}
		case SDL_KEYUP:
			ctx.keys[e.key.keysym.scancode].hit = false;
			ctx.keys[e.key.keysym.scancode].event = e.key;
			break;

		case SDL_MOUSEMOTION:
			ctx.mouse.x = e.motion.x;
			ctx.mouse.y = e.motion.y;
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			ctx.mouse.x = e.motion.x;
			ctx.mouse.y = e.motion.y;
			if(e.button.button == SDL_BUTTON_LEFT)
				ctx.mouse.touched = ctx.mouse.l = e.button.state == SDL_PRESSED;
			else if(e.button.button == SDL_BUTTON_RIGHT)
				ctx.mouse.r = e.button.state == SDL_PRESSED;
			break;

		case SDL_MOUSEWHEEL:
			break;

		case SDL_FINGERMOTION:
			ctx.mouse.x = e.tfinger.x;
			ctx.mouse.y = e.tfinger.y;
			break;
		case SDL_FINGERDOWN:
			ctx.mouse.x = e.tfinger.x;
			ctx.mouse.y = e.tfinger.y;
			ctx.mouse.touched = ctx.mouse.l = true;
			break;
		case SDL_FINGERUP:
			ctx.mouse.x = e.tfinger.x;
			ctx.mouse.y = e.tfinger.y;
			ctx.mouse.touched = ctx.mouse.l = false;
			break;

		case SDL_WINDOWEVENT:
		{
			switch(e.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				ctx.w = e.window.data1;
				ctx.h = e.window.data2;
				bgfx_reset(ctx.w, ctx.h, ctx.reset_flags);
				break;
			case SDL_WINDOWEVENT_CLOSE:
				break;
			default:
				break;
			}
			break;
		}

		default:
			break;
		}
	}
	return true;
}

uint16_t w_width()  {return ctx.w;}
uint16_t w_height() {return ctx.h;}

void w_dbg(uint32_t options)
{
	bgfx_set_debug(BGFX_DEBUG_NONE
				| (options & DBG_TEXT      ? BGFX_DEBUG_TEXT      : 0)
				| (options & DBG_WIREFRAME ? BGFX_DEBUG_WIREFRAME : 0)
				| (options & DBG_STATS     ? BGFX_DEBUG_STATS     : 0)
	);
}

float w_mx() {return ctx.mouse.x;}
float w_my() {return ctx.mouse.y;}
bool w_mtouch() {return ctx.mouse.touched;}
bool w_ml() {return ctx.mouse.l;}
bool w_mr() {return ctx.mouse.r;}

static SDL_Scancode _w_key(uint8_t key)
{
	if(key >= 'a' && key <= 'z')
		return key - 'a' + SDL_SCANCODE_A;
	else if(key >= '1' && key <= '9')
		return key - '1' + SDL_SCANCODE_1;
	else if(key >= K_PAD1 && key <= K_PAD9)
		return key - K_PAD1 + SDL_SCANCODE_KP_1;
	else if(key >= K_F1 && key <= K_F12)
		return key - K_F1 + SDL_SCANCODE_F1;

	switch(key)
	{
	case '0':			return SDL_SCANCODE_0;
	case K_PAD0:		return SDL_SCANCODE_KP_0;
	case K_PADMUL:		return SDL_SCANCODE_KP_MULTIPLY;
	case K_PADADD:		return SDL_SCANCODE_KP_PLUS;
	case K_PADENTER:	return SDL_SCANCODE_KP_ENTER;
	case K_PADSUB:		return SDL_SCANCODE_KP_MINUS;
	case K_PADDOT:		return SDL_SCANCODE_KP_PERIOD;
	case K_PADDIV:		return SDL_SCANCODE_KP_DIVIDE;
	case K_BACKSPACE:	return SDL_SCANCODE_BACKSPACE;
	case K_TAB:			return SDL_SCANCODE_TAB;
	case K_RETURN:		return SDL_SCANCODE_RETURN;
	case K_PAUSE:		return SDL_SCANCODE_PAUSE;
	case K_ESCAPE:		return SDL_SCANCODE_ESCAPE;
	case K_SPACE:		return SDL_SCANCODE_SPACE;
	case K_PAGEUP:		return SDL_SCANCODE_PAGEUP;
	case K_PAGEDN:		return SDL_SCANCODE_PAGEDOWN;
	case K_END:			return SDL_SCANCODE_END;
	case K_HOME:		return SDL_SCANCODE_HOME;
	case K_LEFT:		return SDL_SCANCODE_LEFT;
	case K_UP:			return SDL_SCANCODE_UP;
	case K_RIGHT:		return SDL_SCANCODE_RIGHT;
	case K_DOWN:		return SDL_SCANCODE_DOWN;
	case K_INSERT:		return SDL_SCANCODE_INSERT;
	case K_DELETE:		return SDL_SCANCODE_DELETE;
	case K_LWIN:		return SDL_SCANCODE_LGUI;
	case K_RWIN:		return SDL_SCANCODE_RGUI;
	case K_SCROLL:		return SDL_SCANCODE_SCROLLLOCK;
	case K_SEMICOLON:	return SDL_SCANCODE_SEMICOLON;
	case K_EQUALS:		return SDL_SCANCODE_EQUALS;
	case K_COMMA:		return SDL_SCANCODE_COMMA;
	case K_MINUS:		return SDL_SCANCODE_MINUS;
	case K_DOT:			return SDL_SCANCODE_PERIOD;
	case K_SLASH:		return SDL_SCANCODE_SLASH;
	case K_BACKTICK:	return SDL_SCANCODE_GRAVE;
	case K_LSQUARE:		return SDL_SCANCODE_LEFTBRACKET; // ?
	case K_RSQUARE:		return SDL_SCANCODE_RIGHTBRACKET;
	case K_BACKSLASH:	return SDL_SCANCODE_BACKSLASH;
	case K_TICK:		return SDL_SCANCODE_APOSTROPHE;
	case K_LSHIFT:		return SDL_SCANCODE_LSHIFT;
	case K_RSHIFT:		return SDL_SCANCODE_RSHIFT;
	case K_LALT:		return SDL_SCANCODE_LALT;
	case K_RALT:		return SDL_SCANCODE_RALT;
	case K_LCONTROL:	return SDL_SCANCODE_LCTRL;
	case K_RCONTROL:	return SDL_SCANCODE_RCTRL;
	case K_CAPSLOCK:	return SDL_SCANCODE_CAPSLOCK;
	case K_NUMLOCK:		return SDL_SCANCODE_NUMLOCKCLEAR;
	default:			return SDL_SCANCODE_UNKNOWN;
	}
}

bool w_k_down(uint8_t key)
{
	return ctx.keys[_w_key(key)].event.state == SDL_PRESSED;
}

bool w_k_hit(uint8_t key)
{
	return ctx.keys[_w_key(key)].hit;
}

