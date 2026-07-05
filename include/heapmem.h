#ifndef CLIB_HEAPMEM_H
#define CLIB_HEAPMEM_H

#include <stddef.h>

void* heap_alloc(size_t size);
void heap_free(void* p);
void* heap_calloc(size_t n, size_t size);
void* heap_realloc(void* p, size_t size);

#endif
