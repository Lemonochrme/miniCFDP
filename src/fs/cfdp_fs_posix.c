#include "cfdp_fs.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdint.h>

int cfdp_fs_open(const char *filename, int write_mode) {
    if (!write_mode) {
        // Open file for reading
        return open(filename, O_RDONLY);
    } else {
        // Open file for writing
        return open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666); // 0666 : rwx rwx rwx
    }
}

int cfdp_fs_read(int fh, void *buffer, size_t size) {
    if (fh < 0) return -1;
    ssize_t ret = read(fh, buffer, size);
    if (ret < 0) {
        return -1;
    }
    return (int)ret;
}

int cfdp_fs_write(int fh, const void *buffer, size_t size) {
    if (fh < 0) return -1;

    ssize_t ret = write(fh, buffer, size);
    return (ret < 0) ? -1 : (int)ret;
}

int cfdp_fs_close(int fh) {
    if (fh < 0) return -1;
    return close(fh);
}

int cfdp_fs_delete(const char *filename) {
    return unlink(filename);
}

int cfdp_fs_rename(const char *old_name, const char *new_name) {
    return rename(old_name, new_name);
}

uint64_t cfdp_fs_size(int fh) {
    if (fh < 0) return 0;

    // save current position
    off_t current = lseek(fh, 0, SEEK_CUR);
    if (current < 0) return 0;

    // Seek end of file
    off_t end = lseek(fh, 0, SEEK_END);
    if (end < 0) return 0;

    // back to initial position
    lseek(fh, current, SEEK_SET);

    return (uint64_t)end;
}