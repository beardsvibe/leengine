#include <window.h>
#include <render.h>
#include <spine.h>
#include <sound.h>

tex_t test;

void init(void * notused)
{
	w_init("00-helloworld", 1000, 700);
	w_dbg(DBG_TEXT);
	test = r_load("some_invalid_random_file_which_shouldn't exists.png", TEX_FLAGS_NONE); // so we get the missing texture
}

void deinit(void * notused)
{
	r_free(test);
	w_deinit();
}

void update(uint16_t w, uint16_t h, float dt, void * notused)
{
	if(w_k_down(K_ESCAPE)) // TODO only for debug
		w_stop();

	static bool dbg_stats = false;
	if(w_k_hit(K_F1))
	{
		dbg_stats = !dbg_stats;
		w_dbg(DBG_TEXT | (dbg_stats ? DBG_STATS : 0));
	}
}

void render(uint16_t w, uint16_t h, float dt, void * notused)
{
	r_viewport(w, h);

	bgfx_dbg_text_clear(0, false);
	bgfx_dbg_text_printf(1, 0, 0x0f, "dt %ims", (uint32_t)(dt * 1000.0f));
	bgfx_dbg_text_printf(1, 1, 0x0f, "%6.1f %6.1f %i", w_mx(), w_my(), w_mtouch());
	bgfx_dbg_text_printf(50, 1, 0x0f, "why, hello there!");

	static float deg = 0.0f;
	deg += 360.0f * dt;
	r_render(test, 0.0f, 0.0f, deg);
}

#include <SDL.h> // for main define
int main(int argc, char * argv[])
{
	return w_run_loop(init, deinit, update, render, NULL);
}
