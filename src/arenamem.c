// SPDX-License-Identifier: MIT

#include "arenamem.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdlib.h>

#define ALIGN_UP(num, align) (((num) + ((align) - 1)) & ~((align) - 1))

// creates a new allocator with the size len
ArenaAlloc ArenaAlloc_new(size_t len) {
	struct ArenaAlloc_s* aas = (struct ArenaAlloc_s*)malloc(sizeof(struct ArenaAlloc_s));
	if (aas == NULL) return NULL;
	aas->cap = len;
	aas->index = 0;
	aas->arena = (char*)malloc(len);
	if (aas->arena == NULL){
		free(aas);
		return NULL;
	}
	aas->next = NULL;
	aas->head = NULL;
	return aas;
}

ArenaAlloc ArenaAlloc_append(ArenaAlloc arena, size_t len) {
	struct ArenaAlloc_s* new_aas = ArenaAlloc_new(len);
	// the first arena doesnt have head set as it is the head
	// every new arena can just copy the head of the last one if it has it
	if (arena->head == NULL)
		new_aas->head = arena;
	else
	 	new_aas->head = arena->head;
	arena->next = new_aas;
	return new_aas;
}

// returns a pointer to a block or memory which length is size
void* ArenaAlloc_alloc(ArenaAlloc* arenaAlloc, size_t size, size_t alignment) {
	ArenaAlloc cur_aas = *arenaAlloc;
	// we allow the user to allign the allocation to a byte amount in case they know what alignment to use
	size_t aligned;
	if (alignment != 0)
		aligned = ALIGN_UP(cur_aas->index, alignment);
	else
	 	aligned = ALIGN_UP(cur_aas->index, alignof(max_align_t));
	// substraction because we dont want to overflow if we do size + aligned index
	if (size > cur_aas->cap - aligned) {
		if (cur_aas->next == NULL) {
			ArenaAlloc new_aas;
			if (cur_aas->cap*2 >= size)
				new_aas = ArenaAlloc_append(cur_aas, cur_aas->cap*2);
			else
		 		new_aas = ArenaAlloc_append(cur_aas, size);

			if (new_aas == NULL) return NULL;
			*arenaAlloc = new_aas;
			cur_aas = new_aas;
		} else {
			if (cur_aas->next->cap < size) {
				ArenaAlloc startskip;
				ArenaAlloc endskip;
				ArenaAlloc target = NULL;
				startskip = cur_aas->next;
				endskip = startskip;
				while (endskip->next != NULL) {
					if (endskip->next->cap < size)
						endskip = endskip->next;
					else {
					 	target = endskip->next;
						break;
					}
				}
				if (target == NULL) {
					if (endskip->cap*2 >= size)
						target = ArenaAlloc_append(endskip, endskip->cap*2);
					else
					 	target = ArenaAlloc_append(endskip, size);
				}
				endskip->next = target->next;
				target->next = startskip->next;
				cur_aas->next = target;
				*arenaAlloc = target;
				cur_aas = target;
			} else {
				*arenaAlloc = cur_aas->next;
				cur_aas = *arenaAlloc;
			}
		}
	}
	void* useralloc = cur_aas->arena + cur_aas->index;
	cur_aas->index += size;
	return useralloc;
}

// resets the current index to the start of the block and reset the current arena to the first one
void ArenaAlloc_reset(ArenaAlloc* arenaAlloc) {
	struct ArenaAlloc_s* cur_aas = *arenaAlloc;
	if (cur_aas->head != NULL) {
		for (struct ArenaAlloc_s* a = cur_aas->head; a != NULL; a = a->next)
			a->index = 0;
		*arenaAlloc = cur_aas->head;
	} else {
		cur_aas->index = 0;
	}
}

// destroys every arena that is referenced in the arenaAlloc using the next pointer
void ArenaAlloc_destroy(ArenaAlloc arenaAlloc) {
	struct ArenaAlloc_s* head = arenaAlloc->head;
	while (head != NULL) {
		struct ArenaAlloc_s* new_aas = head->next;
		free(head->arena);
		free(head);
		head = new_aas;
	}
}
