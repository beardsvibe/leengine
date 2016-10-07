// small helper to represent yaml in more fashionable manner

#pragma once

#include <khash.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct dict_t dict_t;
typedef khint_t diter_t;

KHASH_MAP_INIT_STR(dict_map, dict_t*)

// TODO well internal memory consumption is shit here
// TODO probably we can just do one huge blob and point to it
// TODO but guess it's fine like this for now
struct dict_t
{
	char * value;
	dict_t ** array;
	kh_dict_map_t * dict;
	size_t count;
};

// parse, free, debug
dict_t *     dparsey(const char * yaml_filename);
dict_t *     dparsejs(const char * json_string);
void         dfree(dict_t * dict);
void         dtraverse(dict_t * dict, int level);

// get value by key or index
dict_t *     dget(dict_t * dict, const char * key);
dict_t *     dgeti(dict_t * dict, size_t index);

// iterator
diter_t      dibegin(dict_t * dict);
diter_t      diend(dict_t * dict);
const char * dikey(dict_t * dict, diter_t index); // might return NULL if bucket is empty
dict_t *     divalue(dict_t * dict, diter_t index);

// get value
const char * dstr(dict_t * dict, const char * default_value);
int          dint(dict_t * dict, int default_value);
uint64_t     duint64(dict_t * dict, uint64_t default_value);
uint32_t     duint32(dict_t * dict, uint32_t default_value);
float        dfloat(dict_t * dict, float default_value);

// get value by key
const char * dgstr(dict_t * dict, const char * key, const char * default_value);
int          dgint(dict_t * dict, const char * key, int default_value);
uint32_t     dguint32(dict_t * dict, const char * key, uint32_t default_value);
uint64_t     dguint64(dict_t * dict, const char * key, uint64_t default_value);
float        dgfloat(dict_t * dict, const char * key, float default_value);
bool         dgstr2(char * buffer, size_t buffer_length, dict_t * dict, const char * key, const char * default_value);
