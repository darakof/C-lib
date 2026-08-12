#ifndef CLIB_ARRAY_H
#define CLIB_ARRAY_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct arr_s {
	uint64_t len, cap;
	size_t stride;
	char data[];
};

typedef char* arr;

#define GET_ARR_S_HDR(a) ((struct arr_s*)((a)-(sizeof(struct arr_s))))
#define arr_push(array, value) \
	((array) = _arr_push((array), (value)))
// allocates a new array with the specialized amount of free space
arr arr_new(uint64_t initlen, size_t elemSize);
// initializes a new array using the data from a C array
arr arr_fromcarr(void* data, size_t elemSize, uint64_t len);
// duplicates an array
arr arr_dup(arr array);
// frees the memory of the array
void arr_destroy(arr array);

// makes the array one element longer and writes the data as the new elemnt, assumes the new data is the same size as the other elements
arr _arr_push(arr array, void* data);
// returns a pointer to a new element so the user can set it manually
void* arr_next(arr array);
// returns a pointer to the last element and makes the array one elemnt shorter
void* arr_pop(arr array);

// allocates exactly this much space
arr arr_resize(arr array, uint64_t newlen);
// allocated this much more space
arr arr_reserve(arr array, uint64_t len);
// double the allocated space
arr arr_grow(arr array);
// shrink until no extra space is left
arr arr_shrink(arr array);
// make sure an array has len amount of elemnts free 
arr arr_ensurefree(arr array, uint64_t len);

// initializes every element to 0
void arr_clear(arr array);
// initializes every element to a specific value assumes the value is the same size as an array element
void arr_purge(arr array, void* initData);

// get the length of the array
uint64_t arr_len(arr array);
// get the max length of the array
uint64_t arr_cap(arr array);
// get the element size of the array
size_t arr_elemSize(arr array);
// returns a pointer to an element with bounds checking
void* arr_at(arr array, uint64_t index);
// validates the index of the array
bool arr_indexvalid(arr array, uint64_t index);

// arr views
struct arrView {
	uint64_t offset;
	uint64_t len;
	// needed if we want to be able to move the view inside the array without requiring the array to be passed as an argument
	size_t elemSize;
	uint8_t* data;
};

typedef struct arrView arrView;

arrView* arrv_new(arr array, uint64_t offset, uint64_t len);

// moves the view inside the array to the new offset and len
void arrv_move(arrView* view, uint64_t offset, uint64_t len);
// uses an existing view to not reallocate memory when the old view isnt needed
void arrv_reloc(arrView* view, arr array, uint64_t offset, uint64_t len);

void arrv_destroy(arrView* view);

#endif
