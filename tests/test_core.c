#include "../src/ui/cfdp_ui.h"
#include "../src/mib/cfdp_mib.h"
#include "../src/fs/cfdp_fs.h"
#include "../src/core/cfdp_core.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TMP_SRC_FILE "build/test_src_file.txt"
#define TMP_DST_FILE "recv_file.dat"
#define TEST_CONTENT "CFDP TEST CONTENT"
#define ENTITY_A 1
#define ENTITY_B 2

static int event_called = 0;
static CfdpStatus last_status = -1;

void test_callback(uint32_t transaction_id, CfdpEvent event, CfdpStatus status) {
    if (event == CFDP_EVENT_TRANSACTION_FINISHED) {
        event_called = 1;
        last_status = status;
    }
}

void prepare_test_file() {
    FILE *f = fopen(TMP_SRC_FILE, "w");
    assert(f);
    fputs(TEST_CONTENT, f);
    fclose(f);
}

void verify_file_contents(const char *filename, const char *expected_content) {
    FILE *f = fopen(filename, "r");
    assert(f);
    char buffer[256] = {0};
    fread(buffer, 1, sizeof(buffer), f);
    fclose(f);
    assert(strcmp(buffer, expected_content) == 0);
}

void test_cfdp_integration() {
    cfdp_register_callback(test_callback);

    // Configure MIB for ENTITY_A
    cfdp_mib.local.entity_id = ENTITY_A;
    cfdp_mib.remote_entity_count = 1;
    cfdp_mib.remote_entities[0].entity_id = ENTITY_B;
    strcpy(cfdp_mib.remote_entities[0].addr, "127.0.0.1");
    cfdp_mib.remote_entities[0].port = 0; // Not used in this test

    cfdp_init();

    prepare_test_file();

    int tid = cfdp_request_put(ENTITY_B, TMP_SRC_FILE, "ignored_dest_name", false);
    assert(tid > 0);
    assert(event_called == 1);
    assert(last_status == CFDP_STATUS_SUCCESS);

    // Simulate receiving the same file back (normally done by another node)
    FILE *f = fopen(TMP_SRC_FILE, "r");
    assert(f);
    char buffer[1024];
    size_t len = fread(buffer, 1, sizeof(buffer), f);
    fclose(f);

    event_called = 0;
    last_status = -1;

    cfdp_process_pdu((uint8_t*)buffer, len, ENTITY_A);

    assert(event_called == 1);
    assert(last_status == CFDP_STATUS_SUCCESS);

    verify_file_contents(TMP_DST_FILE, TEST_CONTENT);

    cfdp_shutdown();

    unlink(TMP_SRC_FILE);
    unlink(TMP_DST_FILE);
}

int main() {
    test_cfdp_integration();
    return 0;
}
