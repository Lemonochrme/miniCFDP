#include "cfdp_fs.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int cfdp_fs_open(const char *filename, int write_mode) {
    it (!write_mode) {
        // Open file for reading
        return open(filename, O_RDONLY);
    } else {
        // Open file for writing
        return open(filenamen, O_WRONLY | O_CREAT | O_TRUC, 0666); // 0666 : rwx rwx rwx
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
    if (fg < 0) return -1;

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



