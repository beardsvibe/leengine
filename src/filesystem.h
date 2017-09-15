#pragma once

// -----------------------------------------------------------------------------

#include <stdio.h>

// please use this instead of fopen
// because files might be in a different place than you expect
// on some platforms this might end up doing fmemopen or similar
FILE * fsopen(const char * filename, const char * mode);

// TODO remove this
void _fs_path(const char * filename, char * buf, size_t size);

// TODO add iOS specific things for file writing, like context, lifetime, etc

// TODO proper game save interface
FILE * fsopen_gamesave(const char * filename, const char * mode);

// -----------------------------------------------------------------------------
// helpers for 3rdparty tools

#include <stb_image.h>
STBIDEF stbi_uc * stbi_fsload(char const * filename, int * x, int * y, int * comp, int req_comp);

//#include <stb_image_write.h>
//STBIWDEF int stbi_write_png_fs(char const * filename, int w, int h, int comp, const void * data, int stride_in_bytes);
