// works and is effecient but it feels disgusting, probably a way to make it look better and more readable

#include "poolmem.h"

#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN_UP(num, align) (((num) + ((align) - 1)) & ~((align) - 1))

// create a new pool with (chunks) amount of chunks each being size bytes long
PoolAlloc PoolAlloc_new(uint64_t chunks, size_t size) {
	if (chunks <= 0 || size <= 0) return NULL;
	size = ALIGN_UP(size, alignof(max_align_t));
	if (size != 0 && chunks > SIZE_MAX/size)
		return NULL;
	if (size < sizeof(void*))
		size = sizeof(void*);

	struct PoolAlloc_s* pool = (struct PoolAlloc_s*)malloc(sizeof(struct PoolAlloc_s));
	if (pool == NULL) return NULL;
	pool->chunkSize = size;
	pool->chunkAmount = chunks;

	pool->mem = (char*)malloc(chunks*size);
	if (pool->mem == NULL) {
		free(pool);
		return NULL;
	}

	//  get the amount of bytes needed so every chunk has its own bit in a uint8_t
	const size_t bitmapbytes = ALIGN_UP(chunks, 8)/8;
	// make sure the bitmap is initialized to zeros
	pool->usagemap = (uint8_t*)calloc(bitmapbytes, 1);
	if (pool->usagemap == NULL) {
		free(pool->mem);
		free(pool);
		return NULL;
	}

	// initialize each chunk to hold a pointer to the next chunk
	// the first chunk is the head
	pool->head = pool->mem;
	for (uint64_t i = 0; i < chunks-1; i++) {
		*(char**)(pool->mem + i * size) = pool->mem + (i+1) * size;
	}
	*(char**)(pool->mem + (chunks-1)*size) = NULL;
	
	return pool;
}

// returns a void* to a start of a chunk that isnt used, returns NULL when no space left
void* PoolAlloc_alloc(PoolAlloc pool) {
	if (pool->head == NULL) return NULL;
	char* chunk = pool->head;
	pool->head = *(char**)chunk;

	uint64_t index = ((char*)chunk - pool->mem) / pool->chunkSize;
	size_t byte = index / 8;
	size_t bit = index % 8;
	pool->usagemap[byte] |= (1u << bit);

	return chunk;
}

// returns the chunk back into circulation after being used
void PoolAlloc_free(PoolAlloc pool, void* chunk) {
	assert((char*)chunk >= pool->mem);
	assert((char*)chunk < pool->mem + pool->chunkAmount * pool->chunkSize);

	uintptr_t offset = (char*)chunk - pool->mem;
	assert(offset % pool->chunkSize == 0);
	uint64_t index = offset / pool->chunkSize;

	size_t byte = index / 8;
	size_t bit = index % 8;

	if (!(pool->usagemap[byte] & (1u << bit))) return;
	pool->usagemap[byte] &= ~(1u << bit);

	*(char**)chunk = pool->head;
	pool->head = (char*)chunk;
}

// sets the whole chunk to 0
void PoolAlloc_clearchunk(PoolAlloc pool, void* chunk) {
	memset(chunk, 0, pool->chunkSize);
}

// free the pool making every used chunk invalid
void PoolAlloc_destroy(PoolAlloc pool) {
	if (pool == NULL) return;
	free(pool->usagemap);
	free(pool->mem);
	free(pool);
}
