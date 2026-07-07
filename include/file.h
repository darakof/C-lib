#ifndef CLIB_FILE_H
#define CLIB_FILE_H

#include "_string.h"

#include <stdio.h>

struct FileDesc {
	str name;
	FILE* fd;
	size_t len;
};

#endif
