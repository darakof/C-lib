#include "array.h"
#include <stdlib.h>
#include <string.h>

// allocates a new array with the specialized amount of free space
arr arr_new(uint64_t initlen, size_t elemSize) {
	struct arr_s* array = (struct arr_s*)calloc(1, sizeof(struct arr_s)+(elemSize*initlen));
	array->len = 0;
	array->cap = initlen;
	array->stride = elemSize;
	return array->data;
}

// initializes a new array using the data from a C array
arr arr_fromcarr(void* data, size_t elemSize, uint64_t len) {
	arr array = arr_new(len, elemSize);
	memcpy(array, data, elemSize*len);
	GET_ARR_S_HDR(array)->len = len;
	return array;
}

// duplicate an array
arr arr_dup(arr array) {
	struct arr_s* arrhdr = GET_ARR_S_HDR(array);
	size_t arraySize = sizeof(struct arr_s) + arrhdr->cap*arrhdr->stride;
	struct arr_s* new_arr = (struct arr_s*)calloc(1, arraySize);
	memcpy(new_arr, arrhdr, arraySize);
	return new_arr->data;
}

// frees the memory of the array
void arr_destroy(arr array) {
	free(GET_ARR_S_HDR(array));
}

// makes the array one element longer and writes the data as the new element, assumes the new data is the same size as the other elements
arr _arr_push(arr array, void* data) {
	struct arr_s* arrhdr = GET_ARR_S_HDR(array);
	if (arrhdr->cap < arrhdr->len+1) {
		array = arr_grow(array);
		if (array == NULL) return NULL;
		arrhdr = GET_ARR_S_HDR(array);
	}
	memcpy(array+(arrhdr->len++*arrhdr->stride), data, arrhdr->stride);
	return array;
}

// returns a pointer to a new element so the user can set it manually, NULL if not enough capacity is in the array
void* arr_next(arr array) {
	struct arr_s* arrhdr = GET_ARR_S_HDR(array);
	if (arrhdr->cap < arrhdr->len+1) return NULL;
	return array+(arrhdr->len++*arrhdr->stride);
}

// returns a pointer to the last element and makes the array one elemnt shorter
void* arr_pop(arr array) {
	struct arr_s* arrhdr = GET_ARR_S_HDR(array);
	
	if (arrhdr->len == 0) return NULL;

	// return the element using the predecremented length 
	// (because the length is always one ahead of the last elemnt this ensures it gives
	// the pointer of the last element and the last element is marked as usable) 
	return &array[(--arrhdr->len)*arrhdr->stride];
}

// allocates exactly this much space
arr arr_resize(arr array, uint64_t newlen) {
	struct arr_s* arrhdr = GET_ARR_S_HDR(array);
	arrhdr = (struct arr_s*)realloc(arrhdr, sizeof(struct arr_s*)+(newlen*arrhdr->stride));
	if (arrhdr == NULL) return NULL;
	arrhdr->cap = newlen;
	// make sure the length doesnt exceed the capacity
	if (arrhdr->len > arrhdr->cap) arrhdr->len = arrhdr->cap;
	return arrhdr->data;
}

// allocated this much more space
arr arr_reserve(arr array, uint64_t len) {
	return arr_resize(array, GET_ARR_S_HDR(array)->cap+len);
}

// doubles the allocated space
arr arr_grow(arr array) {
	return arr_reserve(array, GET_ARR_S_HDR(array)->cap);
}

// shrink until no extra space is left
arr arr_shrink(arr array) {
	return arr_resize(array, GET_ARR_S_HDR(array)->len);
}

// makes sure an array has len amount of elements free
arr arr_ensurefree(arr array, uint64_t len) {
	struct arr_s* arrhdr = GET_ARR_S_HDR(array);
	if (arrhdr->cap-arrhdr->len < len)
		array = arr_reserve(array, (arrhdr->len+len)-arrhdr->cap);

	return array;
}

// initializes every element to 0
void arr_clear(arr array) {
	memset(array, 0, GET_ARR_S_HDR(array)->len*GET_ARR_S_HDR(array)->stride);
	GET_ARR_S_HDR(array)->len = 0;
}

// initializes every element to a specific value assumes the value is the same size as an array element
void arr_purge(arr array, void* initData) {
	struct arr_s* arrhdr = GET_ARR_S_HDR(array);
	for (uint64_t i = 0; i < arrhdr->len; i++) {
		memcpy(arrhdr->data+(i*arrhdr->stride), initData, arrhdr->stride);
	}
}

// get the length of the array
uint64_t arr_len(arr array) {
	return GET_ARR_S_HDR(array)->len;
}

// get the max length of the array
uint64_t arr_cap(arr array) {
	return GET_ARR_S_HDR(array)->cap;
}

// get the element size of the array
size_t arr_elemSize(arr array) {
	return GET_ARR_S_HDR(array)->stride;
}

// returns a pointer to an element with bounds checking
void* arr_at(arr array, uint64_t index) {
	return arr_indexvalid(array, index) ? array + arr_elemSize(array)*index : NULL;
}

// validates the index of the array
inline bool arr_indexvalid(arr array, uint64_t index) {
	return index >= 0 && index < arr_len(array) ? true : false;
}
