
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

bool startswith(const char * string_starts, const char * with_prefix)
{
	size_t ls = strlen(string_starts);
	size_t lp = strlen(with_prefix);
	return (ls >= lp) && (memcmp(string_starts, with_prefix, lp) == 0);
}

bool endswith(const char * string_ends, const char * with_prefix)
{
	size_t ls = strlen(string_ends);
	size_t lp = strlen(with_prefix);
	return (ls >= lp) && (memcmp(string_ends + ls - lp, with_prefix, lp) == 0);
}

#ifdef _WIN32
errno_t memset_s(void *v, rsize_t smax, int c, rsize_t n)
{
	if(v == NULL)
		return EINVAL;
	if(n > smax)
		return EINVAL;

	volatile uint8_t * p = v;
	while(smax-- && n--)
		*p++ = c;

	return 0;
}
#endif


#if defined(BIG_ENDIAN) // TODO fix this when we ever compile to big endian platform

uint16_t read_bigendian_16(const uint8_t * i)
{
	return *(const uint16_t*)i;
}

void write_bigendian_16(uint16_t i, uint8_t * o)
{
	*(uint16_t*)o = i;;
}

#else

uint16_t read_bigendian_16(const uint8_t * i)
{
	uint16_t t;
	((uint8_t*)&t)[0] = i[1];
	((uint8_t*)&t)[1] = i[0];
	return t;
}

void write_bigendian_16(uint16_t i, uint8_t * o)
{
	o[0] = ((uint8_t*)&i)[1];
	o[1] = ((uint8_t*)&i)[0];
}

#endif

#if defined(__ANDROID__) || defined(EMSCRIPTEN)
int memset_s(void *v, size_t smax, int c, size_t n)
{
	if(v == NULL)
		return -1;
	if(smax > SIZE_MAX)
		return -1;
	if(n > smax)
		return -1;

	volatile unsigned char * p = v;
	while (smax-- && n--)
	{
		*p++ = c;
	}

	return 0;
}
#endif
