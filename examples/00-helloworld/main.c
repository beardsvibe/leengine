#include <window.h>
#include <render.h>
#include <spine.h>
#include <sound.h>

tex_t test;

int32_t game_init(int32_t argc, char * argv[])
{
	w_dbg(DBG_TEXT);
	test = r_load("some_invalid_random_file_which_shouldn't exists.png", TEX_FLAGS_NONE); // so we get the missing texture
	return 0;
}

int32_t game_deinit()
{
	r_free(test);
	return 0;
}

int32_t game_might_unload()
{
	return 0;
}

int32_t game_update(uint16_t w, uint16_t h, float dt)
{
	if(ep_kdown(EK_ESCAPE)) // TODO only for debug
		return 1;

	static bool dbg_stats = false;
	if(ep_khit(EK_F1))
	{
		dbg_stats = !dbg_stats;
		w_dbg(DBG_TEXT | (dbg_stats ? DBG_STATS : 0));
	}

	return 0;
}

int32_t game_render(uint16_t w, uint16_t h, float dt)
{
	r_viewport(0, 0, w, h, r_to_color(32, 32, 32, 255));

	bgfx_dbg_text_clear(0, false);
	bgfx_dbg_text_printf(1, 0, 0x0f, "dt %ims", (uint32_t)(dt * 1000.0f));
	bgfx_dbg_text_printf(1, 1, 0x0f, "%6.1f %6.1f %i", w_mx(), w_my(), w_mtouch());
	bgfx_dbg_text_printf(50, 1, 0x0f, "why, hello there!");

	static float deg = 0.0f;
	deg += 360.0f * dt;
	r_render(test, 0.0f, 0.0f, deg);

	return 0;
}
