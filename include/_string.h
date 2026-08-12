// SPDX-License-Identifier: MIT

#ifndef STRING_H
#define STRING_H

#include <endian.h>
#include <stddef.h>
#include <stdint.h>

struct str_s {
	uint64_t len, cap;
	char data[];
};

typedef char* str;

#define GET_STR_S_HDR(s) ((struct str_s*)((s)-(sizeof(struct str_s))))
#define str_push(string, c) \
	((string) = _str_push((string), (c));

// create a new blank string with initlen capacity
str str_new(uint64_t initlen);
// create a new string fromt he data
str str_fromcstrlen(char* data, uint64_t len);
// create a new string with the data, uses strlen to find the length
str str_fromcstr(char* data);
// create an exact copy of the provided string
str str_dup(str string);
// free the memory used by string
void str_destroy(str string);

// add c to the end of the string
str _str_push(str string, char c);
// return the last character and replace it with a null terminator (\0)
char str_pop(str string);
// append the C-string to the end of string
str str_appendcstrlen(str string, char* data, uint64_t datalen);
// append the C-string to the end of string, uses strlen to get datalen
str str_appendcstr(str string, char* data);
// append a library string to the end of string
str str_appendstr(str string, str data);

// resizes strings capacity to newlen
str str_resize(str string, uint64_t newlen);
// add len amount of space fo the capacity of string
str str_reserve(str string, uint64_t len);
// doubles the capacity of string
str str_grow(str string);
// makes sure theres enough unused space in a string to fit len amount of characters
str str_ensurefree(str string, uint64_t len);
// reallocates to the length of the string
str str_shrink(str string);

// sets every character in a string to a null terminator (\0)
void str_clear(str string);
// set every character in a string to a specified value
void str_purge(str string, char val);

// get string length as known by the library
uint64_t str_len(str string);
// get the string max capacity
uint64_t str_cap(str string);


// very bad but good enough
struct strView {
	// offset from the begining of the string allows us to move the view back by recalculating the new data pointer
	uint64_t offset;
	// doesnt limit the actual length of data
	uint64_t len;
	// pointer to the view
	char* data; // sadly only valid until it is reallocated since the only way to keep it valid is having a pointer to the function variable which holds the actual string poitner and that has lifetime issues
};

typedef struct strView strView;

strView* strv_new(str string, uint64_t offset, uint64_t len);

// changes the offset and length of the view inside the same string
void strv_move(strView* view, uint64_t offset, uint64_t len);
// allows you to use the same allocated string view on another string, replaces all existing data inside the view
void strv_reloc(strView* view, str string, uint64_t offset, uint64_t len);

void strv_destroy(strView* view);



#endif //  STRING_H
