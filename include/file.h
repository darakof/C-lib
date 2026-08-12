// SPDX-License-Identifier: MIT

#ifndef CLIB_FILE_H
#define CLIB_FILE_H

#include "_string.h"
#include "array.h"

#include <stdio.h>

struct FileDesc {
	str name;
	FILE* fd;
	size_t len;
};

struct MemFile {
	str name;
	arr data;
};

typedef struct FileDesc FileDesc;
typedef struct MemFile MemFile;

FileDesc* FileDesc_open(char* name);
void FileDesc_close(FileDesc* file);

uint8_t* FileDesc_read(FileDesc* file, uint64_t offset, uint64_t len);
void FileDesc_writelen(FileDesc* file, uint64_t offset, uint8_t* data, uint64_t len);
void FileDesc_write(FileDesc* file, uint64_t offset, arr data);

MemFile* MemFile_open(char* name);
MemFile* MemFile_reload(MemFile* file);
void MemFile_close(MemFile* file);

bool File_exists(char* name);
uint64_t File_size(char* name);
bool File_rename(char* name, char* newname);
bool File_remove(char* name);
bool File_new(char* name);
#endif
