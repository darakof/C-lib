// SPDX-License-Identifier: MIT

#ifndef CLIB_POOLMEM_H
#define CLIB_POOLMEM_H

#include <stddef.h>
#include <stdint.h>

struct PoolAlloc_s {
	size_t chunkSize;
	uint64_t chunkAmount;
	uint8_t* usagemap;
	char* head;
	char* mem;
};

typedef struct PoolAlloc_s* PoolAlloc;

// create a new pool with (chunks) amount of chunks each being size bytes long
PoolAlloc PoolAlloc_new(uint64_t chunks, size_t size);
// returns a void* to a start of a chunk that isnt used
void* PoolAlloc_alloc(PoolAlloc pool);
// returns the chunk back into circulation after being used
void PoolAlloc_free(PoolAlloc pool, void* chunk);
// sets the whole chunk to 0
void PoolAlloc_clearchunk(PoolAlloc pool, void* chunk);
// free the pool making every used chunk invalid
void PoolAlloc_destroy(PoolAlloc pool);

#endif
