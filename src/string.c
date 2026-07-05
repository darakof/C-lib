#include "_string.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// create a new blank string with initlen capacity
str str_new(uint64_t initlen) {
	struct str_s* strhdr = (struct str_s*)calloc(1, sizeof(struct str_s)+initlen*sizeof(char));
	if (strhdr == NULL) return NULL;
	strhdr->cap = initlen;
	strhdr->len = 0;
	strhdr->data[0] = '\0';
	return strhdr->data;
}

// create a new string fromt he data
str str_fromcstrlen(char* data, uint64_t len) {
	str string = str_new(len);
	if (string == NULL) return NULL;
	memcpy(string, data, len);
	GET_STR_S_HDR(string)->len = len;
	return string;
}

// create a new string with the data, uses strlen to find the length
str str_fromcstr(char* data) {
	return str_fromcstrlen(data, strlen(data));
}

// free the memory used by string
void str_destroy(str string) {
	free(GET_STR_S_HDR(string));
}

// add c to the end of the string
str str_push(str string, char c) {
	struct str_s* strhdr = GET_STR_S_HDR(string);
	if (strhdr->cap < strhdr->len+1) {
		string = str_grow(string);
		if (string == NULL) return NULL;
		strhdr = GET_STR_S_HDR(string);
	}
	string[strhdr->len++] = c;
	string[strhdr->len] = '\0';
	return string;
}

// return the last character and replace it with a null terminator (\0)
char str_pop(str string) {
	struct str_s* strhdr = GET_STR_S_HDR(string);
	char c = string[--strhdr->len];
	string[strhdr->len] = '\0';
	return c;
}

// append the C-string to the end of string, datalen doesnt include null terminator
str str_appendcstrlen(str string, char* data, uint64_t datalen) {
	struct str_s* strhdr = GET_STR_S_HDR(string);
	if (strhdr->cap < strhdr->len+datalen) {
		string = str_reserve(string, (strhdr->len+datalen+1)-strhdr->cap);
		if (string == NULL) return NULL;
		strhdr = GET_STR_S_HDR(string);
	}

	memcpy(string+strhdr->len, data, datalen);
	strhdr->len += datalen;
	string[strhdr->len] = '\0';
	return string;
}

// append the C-string to the end of string, uses strlen to get datalen
str str_appendcstr(str string, char* data) {
	return str_appendcstrlen(string, data, strlen(data));
}

// append a library string to the end of string
str str_appendstr(str string, str data) {
	return str_appendcstrlen(string, data, GET_STR_S_HDR(data)->len);
}

// resizes strings capacity to newlen
str str_resize(str string, uint64_t newlen) {
	struct str_s* strhdr = GET_STR_S_HDR(string);
	strhdr = (struct str_s*)realloc(strhdr, sizeof(struct str_s)+(newlen+1)*sizeof(char));
	if (strhdr == NULL) return NULL;
	strhdr->cap = newlen;
	if (strhdr->cap < strhdr->len) strhdr->len = strhdr->cap;
	strhdr->data[strhdr->len] = '\0';
	return strhdr->data;
}

// add len amount of space fo the capacity of string
str str_reserve(str string, uint64_t len) {
	return str_resize(string, GET_STR_S_HDR(string)->cap+len);
}

// doubles the capacity of string
str str_grow(str string) {
	return str_resize(string, GET_STR_S_HDR(string)->cap*2);
}

// makes sure theres enough unused space in a string to fit len amount of characters
str str_ensurefree(str string, uint64_t len) {
	struct str_s* strhdr = GET_STR_S_HDR(string);
	if (strhdr->cap < strhdr->len+len+1) {
		string = str_reserve(string, (strhdr->len + len + 1)-strhdr->cap);
		if (string == NULL) return NULL;
		strhdr = GET_STR_S_HDR(string);
	}
	return string;
}

// reallocates to the length of the string
str str_shrink(str string) {
	return str_resize(string, GET_STR_S_HDR(string)->len+1);;
}

// sets every character in a string to a null terminator (\0)
void str_clear(str string) {
	str_purge(string, '\0');
}

// set every character in a string to a specified value
void str_purge(str string, char val) {
	memset(string, val, GET_STR_S_HDR(string)->len);
}

// get string length as known by the library
uint64_t str_len(str string) {
	return GET_STR_S_HDR(string)->len;
}

// get the string max capacity
uint64_t str_cap(str string) {
	return GET_STR_S_HDR(string)->cap;
}
