
#pragma once

#include <uuuid2.h>
#include <khash.h>
#include "murmur3hash.h"

#define kh_uuid_hash_func(key) murmur16((uint32_t*)(key).bytes)
#define kh_uuid_hash_equal(a, b) uuuid2_eq((a), (b))
#define KHASH_MAP_INIT_UUID(name, khval_t) KHASH_INIT(name, uuuid2_t, khval_t, 1, kh_uuid_hash_func, kh_uuid_hash_equal)
