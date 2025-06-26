#include "../src/fs/cfdp_fs.h"
#include <string.h>
#include <assert.h>

#define TEST_FILENAME "test_file.txt"
#define RENAMED_FILENAME "renamed_file.txt"

void test_fs_write_read_close() {
    const char *data = "Hello CFDP!";
    char buffer[128] = {0};

    int fd = cfdp_fs_open(TEST_FILENAME, 1); // Write mode
    assert(fd >= 0);

    int bytes_written = cfdp_fs_write(fd, data, strlen(data));
    assert(bytes_written == (int)strlen(data));
    assert(cfdp_fs_close(fd) == 0);

    fd = cfdp_fs_open(TEST_FILENAME, 0); // Read mode
    assert(fd >= 0);

    int bytes_read = cfdp_fs_read(fd, buffer, sizeof(buffer) - 1);
    assert(bytes_read == (int)strlen(data));
    buffer[bytes_read] = '\0';
    assert(strcmp(data, buffer) == 0);
    assert(cfdp_fs_close(fd) == 0);
}

void test_fs_rename() {
    assert(cfdp_fs_rename(TEST_FILENAME, RENAMED_FILENAME) == 0);
}

void test_fs_delete() {
    assert(cfdp_fs_delete(RENAMED_FILENAME) == 0);
}

int main() {
    test_fs_write_read_close();
    test_fs_rename();
    test_fs_delete();
    return 0;
}
