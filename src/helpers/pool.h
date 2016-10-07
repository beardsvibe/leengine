#if 0

#pragma once

// can't be bothered, simplest pool wrapper out there

#include <stdint.h>
#include <stdbool.h>
#include <tlsf.h>

typedef struct
{
	void * mem;
	tlsf_t tlsf;
	size_t size;
} pmem_t;

void pinit(pmem_t * pool, size_t object_size, size_t object_count);
void pdestory(pmem_t * pool);
void * palloc(pmem_t * pool);
void pfree(pmem_t * pool, void * ptr);

#ifdef POOL_IMPLEMENTATION

void pinit(pmem_t * pool, size_t object_size, size_t object_count)
{
	size_t size = tlsf_size() + tlsf_align_size() + tlsf_block_size_min();
	size += (object_size + tlsf_pool_overhead() + tlsf_alloc_overhead()) * object_count; // TODO check this
	pool->mem = malloc(size);
	pool->tlsf = tlsf_create_with_pool(pool->mem, size);
	pool->size = object_size;
}

void pdestory(pmem_t * pool)
{
	free(pool->mem);
	memset(pool, 0, sizeof(pmem_t));
}

void * palloc(pmem_t * pool)
{
	return tlsf_malloc(pool->tlsf, pool->size);
}

void pfree(pmem_t * pool, void * ptr)
{
	tlsf_free(pool->tlsf, ptr);
}

#endif POOL_IMPLEMENTATION

#endif
