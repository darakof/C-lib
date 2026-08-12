// SPDX-License-Identifier: MIT

#include "heapmem.h"
#include <stdlib.h>

void* heap_malloc(size_t size) {
	return malloc(size);
}

void heap_free(void* p) {
	free(p);
}

void* heap_calloc(size_t n, size_t size) {
	return calloc(n, size);
}

void* heap_realloc(void* p, size_t size) {
	return realloc(p, size);
}
