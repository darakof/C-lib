#include "file.h"
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

FileDesc FileDesc_open(str name) {
	FileDesc file = (struct FileDesc_s*)malloc(sizeof(struct FileDesc_s));
	file->name = name;
	// allows read/write and creates new file if not existent
	file->fd = fopen(name, "w+");
	if (file->fd == NULL)
		goto FILEDESC_OPEN_FAILURE;
	unsigned long fileLen = 0;
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

void FileDesc_close(FileDesc file) {
	if (file == NULL) return;
	
	if (file->fd != NULL)
		fclose(file->fd);
	if (file->name != NULL)
		str_destroy(file->name);
}

uint8_t* FileDesc_read(FileDesc file, uint64_t len);
void FileDesc_write(FileDesc file, arr data);

MemFile MemFile_open(str name);
void MemFile_close(MemFile file);

bool File_exists(char* name);
uint64_t File_size(char* name);
bool File_rename(char* name, char* newname);
bool File_remove(char* name);
bool File_new(char* name);

