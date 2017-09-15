#include "sound.h"
#include <stdio.h>

#ifdef FMOD_AVAILABLE

#include "filesystem.h"
#include <fmod_studio.h>
#include <fmod_errors.h>

static struct
{
	FMOD_STUDIO_SYSTEM * system;
} ctx;

static void _e(FMOD_RESULT result)
{
	if(result != FMOD_OK)
	{
		fprintf(stderr, "fmod error : %s\n", FMOD_ErrorString(result));
		fflush(stderr);
	}
}

void _s_init()
{
	_e(FMOD_Studio_System_Create(&ctx.system, FMOD_VERSION));
	FMOD_STUDIO_INITFLAGS studio_initflags = FMOD_STUDIO_INIT_NORMAL;
	FMOD_INITFLAGS fmod_initflags = FMOD_INIT_NORMAL;
	#ifdef FMOD_DEBUG
	studio_initflags |= FMOD_STUDIO_INIT_LIVEUPDATE;
	fmod_initflags |= FMOD_INIT_PROFILE_ENABLE;
	#endif
	_e(FMOD_Studio_System_Initialize(ctx.system, 1024, studio_initflags, fmod_initflags, NULL));
}

void _s_deinit()
{
	_e(FMOD_Studio_System_Release(ctx.system));
}

void _s_update()
{
	_e(FMOD_Studio_System_Update(ctx.system));
}

bank_t s_load_bank(const char * path)
{
	bank_t ret;

	char buf[1024 * 2] = {0};
	_fs_path(path, buf, sizeof(buf));

	_e(FMOD_Studio_System_LoadBankFile(ctx.system, buf, FMOD_STUDIO_LOAD_BANK_NORMAL, (FMOD_STUDIO_BANK**)&ret.inst));
	return ret;
}

static FMOD_RESULT fmod_destroy_cb(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE * ev, void * parameters)
{
	(void)parameters;
	s_on_destroyed_t cb = NULL;
	_e(FMOD_Studio_EventInstance_GetUserData(ev, (void**)&cb));
	if(cb && type == FMOD_STUDIO_EVENT_CALLBACK_DESTROYED)
		(*cb)(ev);
	return FMOD_OK;
}

sound_t s_create(const char * event_path, s_on_destroyed_t cb)
{
	sound_t ret;
	FMOD_STUDIO_EVENTDESCRIPTION * ev;
	_e(FMOD_Studio_System_GetEvent(ctx.system, event_path, &ev));
	_e(FMOD_Studio_EventDescription_CreateInstance(ev, (FMOD_STUDIO_EVENTINSTANCE**)&ret.inst));
	if(cb)
	{
		_e(FMOD_Studio_EventInstance_SetUserData((FMOD_STUDIO_EVENTINSTANCE*)ret.inst, (void*)cb));
		_e(FMOD_Studio_EventInstance_SetCallback((FMOD_STUDIO_EVENTINSTANCE*)ret.inst, fmod_destroy_cb, FMOD_STUDIO_EVENT_CALLBACK_DESTROYED));
	}
	return ret;
}

void s_free(sound_t sound)
{
	_e(FMOD_Studio_EventInstance_Release((FMOD_STUDIO_EVENTINSTANCE*)sound.inst));
}

void s_start(sound_t sound)
{
	#ifdef FMOD_DEBUG
	{
		FMOD_STUDIO_EVENTDESCRIPTION * ev = NULL;
		_e(FMOD_Studio_EventInstance_GetDescription((FMOD_STUDIO_EVENTINSTANCE*)sound.inst, &ev));

		char path_temp[512] = {0};
		_e(FMOD_Studio_EventDescription_GetPath(ev, path_temp, sizeof(path_temp), NULL));
		printf("%s: playing '%s'\n", __func__, path_temp);
		fflush(stdout);
	}
	#endif

	_e(FMOD_Studio_EventInstance_Start((FMOD_STUDIO_EVENTINSTANCE*)sound.inst));
}

void s_stop(sound_t sound)
{
	#ifdef FMOD_DEBUG
	{
		FMOD_STUDIO_EVENTDESCRIPTION * ev = NULL;
		_e(FMOD_Studio_EventInstance_GetDescription((FMOD_STUDIO_EVENTINSTANCE*)sound.inst, &ev));

		char path_temp[512] = {0};
		_e(FMOD_Studio_EventDescription_GetPath(ev, path_temp, sizeof(path_temp), NULL));
		printf("%s: stoping '%s'\n", __func__, path_temp);
		fflush(stdout);
	}
	#endif

	_e(FMOD_Studio_EventInstance_Stop((FMOD_STUDIO_EVENTINSTANCE*)sound.inst, FMOD_STUDIO_STOP_ALLOWFADEOUT));
}

void s_set(sound_t sound, const char * parameter, float value)
{
	_e(FMOD_Studio_EventInstance_SetParameterValue((FMOD_STUDIO_EVENTINSTANCE*)sound.inst, parameter, value));
}

bool s_is_playing(sound_t sound)
{
	if(!sound.inst)
		return false;

	FMOD_STUDIO_PLAYBACK_STATE state = 0;
	_e(FMOD_Studio_EventInstance_GetPlaybackState((FMOD_STUDIO_EVENTINSTANCE*)sound.inst, &state));
	return
		state == FMOD_STUDIO_PLAYBACK_PLAYING ||
		state == FMOD_STUDIO_PLAYBACK_SUSTAINING ||
		state == FMOD_STUDIO_PLAYBACK_STARTING;
}

#else

void _s_init() {}
void _s_deinit() {}
void _s_update() {}

bank_t s_load_bank(const char * path)
{
	bank_t ret = {0};
	(void)path;
	return ret;
}

sound_t s_create(const char * event_path, s_on_destroyed_t cb)
{
	sound_t ret = {0};
	(void)event_path;
	return ret;
}

void s_free(sound_t sound) {(void)sound;}
void s_start(sound_t sound) {(void)sound;}
void s_stop(sound_t sound) {(void)sound;}
void s_set(sound_t sound, const char * parameter, float value) {(void)sound; (void)parameter; (void)value;}
bool s_is_playing(sound_t sound) {(void)sound; return false;}

#endif
