#include "sound.h"
#include <stdio.h>

#ifdef FMOD_AVAILABLE

#include <fmod_studio.h>
#include <fmod_errors.h>

static struct
{
	FMOD_STUDIO_SYSTEM * system;
} ctx;

static _e(FMOD_RESULT result)
{
	if(result != FMOD_OK)
		fprintf(stderr, "fmod error : %s\n", FMOD_ErrorString(result));
}

void _s_init()
{
	_e(FMOD_Studio_System_Create(&ctx.system, FMOD_VERSION));
	_e(FMOD_Studio_System_Initialize(ctx.system, 1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, NULL));
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
	_e(FMOD_Studio_System_LoadBankFile(ctx.system, path, FMOD_STUDIO_LOAD_BANK_NORMAL, (FMOD_STUDIO_BANK**)&ret.inst));
	return ret;
}

sound_t s_create(const char * event_path)
{
	sound_t ret;
	FMOD_STUDIO_EVENTDESCRIPTION * ev;
	_e(FMOD_Studio_System_GetEvent(ctx.system, event_path, &ev));
	_e(FMOD_Studio_EventDescription_CreateInstance(ev, (FMOD_STUDIO_EVENTINSTANCE**)&ret.inst));
	return ret;
}

void s_free(sound_t sound)
{
	_e(FMOD_Studio_EventInstance_Release((FMOD_STUDIO_EVENTINSTANCE*)sound.inst));
}

void s_start(sound_t sound)
{
	_e(FMOD_Studio_EventInstance_Start((FMOD_STUDIO_EVENTINSTANCE*)sound.inst));
}

void s_stop(sound_t sound)
{
	_e(FMOD_Studio_EventInstance_Stop((FMOD_STUDIO_EVENTINSTANCE*)sound.inst, FMOD_STUDIO_STOP_ALLOWFADEOUT));
}

void s_set(sound_t sound, const char * parameter, float value)
{
	_e(FMOD_Studio_EventInstance_SetParameterValue((FMOD_STUDIO_EVENTINSTANCE*)sound.inst, parameter, value));
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

sound_t s_create(const char * event_path)
{
	sound_t ret = {0};
	(void)event_path;
	return ret;
}

void s_free(sound_t sound) {(void)sound;}
void s_start(sound_t sound) {(void)sound;}
void s_stop(sound_t sound) {(void)sound;}
void s_set(sound_t sound, const char * parameter, float value) {(void)sound; (void)parameter; (void)value;}

#endif
