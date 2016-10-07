#include "pstrstream.h"
#include <stdio.h>

void ssinit(strstream_t * stream, char * buffer, size_t buffer_size)
{
	stream->buffer = buffer;
	stream->size = buffer_size;
	stream->pos = 0;
}

bool ssvalid(strstream_t * stream)
{
	return stream->pos <= stream->size;
}

bool ssprintf(strstream_t * stream, const char * format, ...)
{
	if(!ssvalid(stream))
		return false;

	va_list args;
	va_start(args, format);
	int result = vsnprintf(stream->buffer + stream->pos, stream->size - stream->pos, format, args);
	va_end(args);

	if(result < 0)
		return false;

	stream->pos += result;

	return ssvalid(stream);
}
