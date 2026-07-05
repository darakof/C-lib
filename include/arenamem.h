#ifndef CLIB_ARENA_ALLOC_H
#define CLIB_ARENA_ALLOC_H

#include <stddef.h>
#include <stdint.h>

struct ArenaAlloc_s {
	uint64_t cap, index;
	char* arena;
	struct ArenaAlloc_s* head;
	struct ArenaAlloc_s* next;
};

typedef struct ArenaAlloc_s* ArenaAlloc;

// creates a new allocator with the size len
ArenaAlloc ArenaAlloc_new(size_t len);
// returns a pointer to a block or memory which length is size
void* ArenaAlloc_alloc(ArenaAlloc* arenaAlloc, size_t size);
// resets the current index to the start of the block and uses the next pointer as the new block if not enough space is left
void ArenaAlloc_reset(ArenaAlloc* arenaAlloc);
// destroys every arena that is referenced in the arenaAlloc using the next pointer
void ArenaAlloc_destroy(ArenaAlloc arenaAlloc);

#endif
