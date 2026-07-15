#ifndef CLIB_FILE_H
#define CLIB_FILE_H

#include "_string.h"
#include "array.h"

#include <stdio.h>

struct FileDesc_s {
	str name;
	FILE* fd;
	size_t len;
};

struct MemFile_S {
	str name;
	arr data;
};

typedef struct FileDesc_s* FileDesc;
typedef struct MemFile_s* MemFile;

FileDesc FileDesc_open(str name);
void FileDesc_close(FileDesc file);

uint8_t* FileDesc_read(FileDesc file, uint64_t len);
void FileDesc_write(FileDesc file, arr data);

MemFile MemFile_open(str name);
void MemFile_close(MemFile file);

bool File_exists(char* name);
uint64_t File_size(char* name);
bool File_rename(char* name, char* newname);
bool File_remove(char* name);
bool File_new(char* name);
#endif
