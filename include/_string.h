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

// create a new blank string with initlen capacity
str str_new(uint64_t initlen);
// create a new string fromt he data
str str_fromcstrlen(char* data, uint64_t len);
// create a new string with the data, uses strlen to find the length
str str_fromcstr(char* data);
// free the memory used by string
void str_destroy(str string);

// add c to the end of the string
str str_push(str string, char c);
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

#endif //  STRING_H
