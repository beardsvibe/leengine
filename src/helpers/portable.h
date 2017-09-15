#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <entrypoint.h>

#if !defined(__FreeBSD__) && !defined(EMSCRIPTEN) && !defined(__APPLE__)

size_t strlcpy(char * dst, const char * src, size_t size);

#endif

#include <sys/stat.h>
#include <fcntl.h>

#ifdef _WIN32
#ifndef stat
#define stat _stat
#endif
#ifndef fstat
#define fstat _fstat
#endif
#ifndef open
#define open _open
#endif
#ifndef close
#define close _close
#endif
#ifndef O_RDONLY
#define O_RDONLY _O_RDONLY
#endif
#ifndef O_RDONLY
#define O_RDONLY _O_RDONLY
#endif
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

// C doesn't have strcmp in prefix form, wtf
bool startswith(const char * string_starts, const char * with_prefix);
bool endswith(const char * string_ends, const char * with_prefix);

#define P_STATIC_ASSERT(__cond, __msg) typedef char static_assertion_##__msg[(__cond) ? 1 : -1]

#ifdef _WIN32
errno_t memset_s(void * v, rsize_t smax, int c, rsize_t n);
#endif

uint16_t read_bigendian_16(const uint8_t * i);
void write_bigendian_16(uint16_t i, uint8_t * o);

#if defined(__ANDROID__) || defined(EMSCRIPTEN)
int memset_s(void *v, size_t smax, int c, size_t n);
#endif

#ifdef _MSC_VER
#ifndef alloca
#define alloca _alloca
#endif
#endif
