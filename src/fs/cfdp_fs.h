#ifndef CFDP_FS_H
#define CFDP_FS_H

#include <stddef.h>

// fh : file handle

// open a file, if write_mode if false open for reading if true open for writing, returns a file handle of -1 on error
int cfdp_fs_open(const char *filename, int write_mode);

// read from an open file handle, returns number of bytes read, 0 on EOF or -1 on error
int cfdp_fs_read(int fh, void *buffer, size_t size);

// write to an open file handle, returns the number of bytes written or -1 on eror
int cfdp_fs_write(int fh, const void *buffer, size_t size);

// close an open file handle
int cfdp_fs_close(int fh);

// delete a file from the filestore
int cfdp_fs_delete(const char *filename);

// rename (move) a file within the filestore
int cfdp_fs_rename(const char *old_name, const char *new_name);


#endif