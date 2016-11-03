
#include "portable.h"

#if !defined(__FreeBSD__) && !defined(EMSCRIPTEN) && !defined(__APPLE__)

size_t strlcpy(char * dst, const char * src, size_t size)
{
	size_t srclen = strlen(src);

	if(!size)
		return srclen + 1;

	size_t copysize = srclen < size - 1 ? srclen : size - 1;
	memcpy(dst, src, copysize);
	dst[copysize] = 0;
	return srclen + 1;
}

#endif
