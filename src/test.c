#include "array.h"
#include "_string.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

int main() {
	// array testing

	char* cstrarr = "Hello, world!\0";
	arr stringarr = arr_new(5, sizeof(char));
	for (int i = 0; i < 14; i++) {
		arr_push(stringarr, &cstrarr[i]);
	}
	arr cstringarr = arr_fromcarr(cstrarr, sizeof(char), 14);
	
	printf("normal c array string: %s\nlen: %"PRIu64"\nmanual array: %s\nlen: %"PRIu64"\ncap: %"PRIu64"\nfrom c string array: %s\nlen: %"PRIu64"\ncap: %"PRIu64"\n", cstrarr, strlen(cstrarr), stringarr, arr_len(stringarr), arr_cap(stringarr), cstringarr, arr_len(cstringarr), arr_cap(cstringarr));

	// string testing
	
	char* cstr = "Hello, world!\0";
	str string = str_new(5);
	for (int i = 0; i < 14; i++) {
		str_push(string, cstr[i]);
	}
	str cstring = str_fromcstrlen(cstr, 14);
	printf("normal c array string: %s\nlen: %"PRIu64"\nmanual string: %s\nlen: %"PRIu64"\ncap: %"PRIu64"\nfrom c string string: %s\nlen: %"PRIu64"\ncap: %"PRIu64"\n", cstr, strlen(cstr), string, str_len(string), str_cap(string), cstring, str_len(cstring), str_cap(cstring));
}
