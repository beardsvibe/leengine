#define ENTRYPOINT_CTX
#include "entrypoint.h"
#include "filesystem.h"
#include "portable.h"

#ifdef EMSCRIPTEN
#include <string.h>
#endif

#if !defined(__APPLE__) || !TARGET_OS_IOS
FILE * fsopen(const char * filename, const char * mode)
{
	#ifdef EMSCRIPTEN
	// TODO remove this hack
	if(memcmp(filename, "res/", 4) == 0)
		filename = filename + 4;
	else
		printf("looks like your file '%s' is not in res, this is not currently supported on emscripten :(\n");
	#endif
	return fopen(filename, mode);
}
void _fs_path(const char * filename, char * buf, size_t size)
{
	strlcpy(buf, filename, size);
}
#endif

STBIDEF stbi_uc * stbi_fsload(char const *filename, int *x, int *y, int *comp, int req_comp)
{
	FILE * f = fsopen(filename, "rb");
	if(!f)
		return NULL;
	stbi_uc * result = stbi_load_from_file(f,x,y,comp,req_comp);
	fclose(f);
	return result;
}

static void _write_to_fsopen(void * context, void * data, int size)
{
	FILE * f = fsopen((const char*)context, "wb");
	if(f)
	{
		fwrite(data, 1, size, f);
		fclose(f);
	}
}

STBIWDEF int stbi_write_png_fs(char const * filename, int w, int h, int comp, const void * data, int stride_in_bytes)
{
	return stbi_write_png_to_func(_write_to_fsopen, (void*)filename, w, h, comp, data, stride_in_bytes);
}
