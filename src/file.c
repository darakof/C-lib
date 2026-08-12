#include "file.h"
#include "_string.h"
#include "string.h"
#include "array.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __unix

#include <sys/stat.h>
#include <unistd.h>

#endif

FileDesc* FileDesc_open(char* name) {
	FileDesc* file = (FileDesc*)malloc(sizeof(FileDesc));
	file->name = str_fromcstr(name);
	// allows read/write and creates new file if not existent
	file->fd = fopen(name, "w+");
	if (file->fd == NULL)
		goto FILEDESC_OPEN_FAILURE;
	signed long long fileLen = 0;
#ifdef __unix
	struct stat fileInfo;
	int fi = fileno(file->fd);

	if (fstat(fi, &fileInfo))
		fileLen = fileInfo.st_size;
	else
		goto FILEDESC_OPEN_FAILURE;
#else
	if (fseek(file->fd, 0, SEEK_END) == 0)
		fileLen = ftell(file->fd);
	else
		goto FILEDESC_OPEN_FAILURE
#endif

	file->len = fileLen;

	goto FILEDESC_OPEN_EXIT;
FILEDESC_OPEN_FAILURE:
	printf("ERROR::FILE::OPEN error string: %s", strerror(errno));
	return NULL;
FILEDESC_OPEN_EXIT:
	return file;
}

void FileDesc_close(FileDesc* file) {
	if (file == NULL) return;
	
	if (file->fd != NULL)
		fclose(file->fd);
	if (file->name != NULL)
		str_destroy(file->name);
}

uint8_t* FileDesc_read(FileDesc* file, uint64_t offset, uint64_t len) {
	if (file == NULL) return NULL;
	if (len == 0) return NULL;
	if (file->len < offset+len) return NULL;
	if (offset != 0) 
		fseek(file->fd, offset, SEEK_SET);
	uint8_t* buf = (uint8_t*)malloc(sizeof(char) * len);
	if (fread(buf, sizeof(char), len, file->fd) != 0)
		return buf;
	else {
		if (feof(file->fd))
			printf("the math is wrong you bum (EOF)");
		else if (ferror(file->fd)) {
			perror("error reading file"); 
		}
	}
}

void FileDesc_writelen(FileDesc* file, uint64_t offset, uint8_t* data, uint64_t len) {
	if (file == NULL) return;
	if (data == NULL) return;
	if (len == 0) return;
	
	if (offset+len > file->len) 
		file->len = offset+len;
	if (offset != 0)
		fseek(file->fd, offset, SEEK_SET);
	
	fwrite(data, sizeof(char), len, file->fd);
}

void FileDesc_write(FileDesc* file, uint64_t offset, arr data) {
	if (data == NULL) return;

	FileDesc_writelen(file, offset, (uint8_t*)data, arr_len(data));
}

MemFile* MemFile_open(char* name) {
	if (name == NULL) return NULL;

	MemFile* file = (MemFile*)malloc(sizeof(MemFile));
	if (file == NULL) return NULL;

	file->name = str_fromcstr(name);
	uint64_t fileLen = File_size(name);
	file->data = arr_new(fileLen, sizeof(char));
	
	FILE* fd = fopen(name, "rb");
	if (fd == NULL) return NULL;

	fread(file->data, sizeof(char), fileLen, fd);
	
	return file;
}

void MemFile_close(MemFile* file) {
	if (file == NULL) return;
	
	// it automatically checks for NULL
	str_destroy(file->name);
	arr_destroy(file->data);
	
	free(file);
}

bool File_exists(char* name) {
#if __unix
	struct stat buffer;

	return (stat(name, &buffer) == 0);
#else
	FILE* fd = fopen(name, "r");
	// file exists
	if (file != NULL) {
		fclose(fd);
		return true;
	}
	// file does not exist
	if (errno == ENOENT) {
		return false;
	}
	// file exists but we dont have read permissions
	return true;
#endif
}

uint64_t File_size(char* name) {
	signed long long fileLen = 0;
#ifdef __unix
	struct stat fileInfo;

	if (stat(name, &fileInfo) == 0)
		fileLen = fileInfo.st_size;
	else
		goto FILESIZE_FAILURE;
#else
	FILE* fd = fopen(name, "rb");
	if (fseek(fd, 0, SEEK_END) == 0)
		fileLen = ftell(fd);
	else
		goto FILESIZE_FAILURE
#endif
	goto FILESIZE_EXIT;

	// the failure first since we want to return 0 either way
FILESIZE_FAILURE:
	printf("ERROR::FILE::OPEN error string: %s", strerror(errno));
FILESIZE_EXIT:
	return fileLen;
}

bool File_rename(char* name, char* newname) {
	if (rename(name, newname)) {
		perror("failed renaming file");
		return false;
	}
	return true;
}

bool File_remove(char* name) {
	if (remove(name)) {
		perror("failed removing file");
		return false;
	}
	return true;
}

bool File_new(char* name) {
	FILE* fd = fopen(name, "w");
	if (fd == NULL) {
		perror("failed creating file");
		return false;
	}
	fclose(fd);
	return true;
}
