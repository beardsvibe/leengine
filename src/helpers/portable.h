#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

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
