#pragma once

// why the f*** fmemopen is not portable? :(

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

typedef struct
{
	char * buffer;
	size_t size;
	size_t pos;
} strstream_t;

void ssinit(strstream_t * stream, char * buffer, size_t buffer_size);
bool ssvalid(strstream_t * stream);

// string stream printf
// returns print on success
bool ssprintf(strstream_t * stream, const char * format, ...);
