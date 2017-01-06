#define ENTRYPOINT_CTX
#include "window.h"
#include "render.h"
#include "sound.h"
#include "physics.h"
#include "text.h"
#include <bgfxplatform.h>
#include <stdio.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

static struct
{
	ep_size_t size;
	uint32_t reset_flags;
	#ifdef ENTRYPOINT_PROVIDE_INPUT
	ep_touch_t touch;
	#endif
} ctx;

int32_t entrypoint_init(int32_t argc, char * argv[])
{
	ctx.size = ep_size();

	#ifdef EMSCRIPTEN
	ctx.reset_flags = BGFX_RESET_NONE;
	#else
	ctx.reset_flags = BGFX_RESET_VSYNC;
	#endif

	bgfx_platform_data_t pd = {0};
	#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	#error TODO
	#elif BX_PLATFORM_IOS
	pd.nwh					= ep_ctx()->caeagllayer;
	#elif BX_PLATFORM_OSX
	pd.nwh					= ep_ctx()->window;
	#elif BX_PLATFORM_WINDOWS
	pd.nwh					= ep_ctx()->hwnd;
	#elif BX_PLATFORM_STEAMLINK
	#error TODO
	#endif
	bgfx_set_platform_data(&pd);

	bgfx_init(BGFX_RENDERER_TYPE_COUNT, BGFX_PCI_ID_NONE, 0, NULL, NULL);

	if(bgfx_get_caps()->supported & BGFX_CAPS_HIDPI)
		ctx.reset_flags |= BGFX_RESET_HIDPI;

	bgfx_reset(ctx.size.w, ctx.size.h, ctx.reset_flags);

	_r_init();
	_s_init();
	_p_init();
	_t_init(1024, 1024);

	return game_init(argc, argv);

//	emscripten_set_main_loop(&w_loop, -1, 1);
}

int32_t entrypoint_deinit()
{
	int32_t err = game_deinit();

	_t_deinit();
	_p_deinit();
	_s_deinit();
	_r_deinit();

	bgfx_shutdown();

	return err;
}

int32_t entrypoint_might_unload()
{
	return game_might_unload();
}

int32_t entrypoint_loop()
{
	// get dt
	#ifdef ENTRYPOINT_PROVIDE_TIME
	float dt = ep_time();
	if(dt > 1.0f / 25.0f)
		dt = 1.0f / 25.0f;
	#else
	float dt = 1.0f / 60.0f;
	#endif

	// handle resizes
	ep_size_t s = ep_size();
	if(s.w != ctx.size.w || s.h != ctx.size.h)
	{
		ctx.size = s;
		bgfx_reset(ctx.size.w, ctx.size.h, ctx.reset_flags);
	}

	// handle touch
	#ifdef ENTRYPOINT_PROVIDE_INPUT
	ep_touch(&ctx.touch);
	#endif

	// update
	int32_t err1 = game_update(ctx.size.w, ctx.size.h, dt);
	_s_update();
	_p_update(dt);

	// render
	int32_t err2 = game_render(ctx.size.w, ctx.size.h, dt);
	//_p_debug_render();
	bgfx_frame(false);

	return (err1 != 0 || err2 != 0) ? 1 : 0;
}

uint16_t w_width()	{return ctx.size.w;}
uint16_t w_height() {return ctx.size.h;}

void w_dbg(uint32_t options)
{
	bgfx_set_debug(BGFX_DEBUG_NONE
				| (options & DBG_TEXT		? BGFX_DEBUG_TEXT		: 0)
				| (options & DBG_WIREFRAME	? BGFX_DEBUG_WIREFRAME	: 0)
				| (options & DBG_STATS		? BGFX_DEBUG_STATS		: 0)
	);
}

#ifdef ENTRYPOINT_PROVIDE_INPUT

float w_mx() {return ctx.touch.x;}
float w_my() {return ctx.touch.y;}
bool w_mtouch() {return ctx.touch.left;}
bool w_ml() {return ctx.touch.left;}
bool w_mr() {return ctx.touch.right;}

uint8_t w_tmax() {return ENTRYPOINT_MAX_MULTITOUCH;}
float w_tx(uint8_t i) {return ctx.touch.multitouch[i].x;}
float w_ty(uint8_t i) {return ctx.touch.multitouch[i].y;}
bool w_touch(uint8_t i) {return ctx.touch.multitouch[i].touched;}

#else

float w_mx() {return 0.0f;}
float w_my() {return 0.0f;}
bool w_mtouch() {return false;}
bool w_ml() {return false;}
bool w_mr() {return false;}

size_t w_tmax() {return 0;}
float w_tx(size_t i) {return 0.0f;}
float w_ty(size_t i) {return 0.0f;}
bool w_touch(size_t i) {return false;}

#endif
