#pragma once

#include <stdbool.h>

typedef struct
{
	void * inst;
} bank_t;

typedef struct
{
	void * inst;
} sound_t;

void _s_init();
void _s_deinit();
void _s_update();

typedef void (*s_on_destroyed_t)(void * inst);

bank_t	s_load_bank(const char * path);
sound_t	s_create(const char * event_path, s_on_destroyed_t callback);
void	s_free(sound_t sound);

void	s_start(sound_t sound);
void	s_stop(sound_t sound);
void	s_set(sound_t sound, const char * parameter, float value);
bool	s_is_playing(sound_t sound);
