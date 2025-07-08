#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "../src/core/cfdp_pdu.h"

int main(void) {
    CfdpPduHeader hdr = {
        .version = CFDP_VERSION,
        .pdu_type = CFDP_PDU_TYPE_FILEDATA,
        .direction = CFDP_DIRECT_TOWARD_RECEIVER,
        .transmission_mode = CFDP_MODE_UNACKED,
        .crc_flag = 0,
        .large_flag_flag = 1,
        .data_field_length = 0x0033,
        .segmentation_control = 0,
        .eid_length = 3,
        .seq_length = 3,
        .source_entity_id = 0xAABBCCDD,
        .transaction_seq_num = 0x01020304,
        .dest_entity_id = 0x11223344
    };

    uint8_t buffer[16] = {0};
    cfdp_serialize_header(buffer, &hdr);

    uint8_t expected[16] = {
        0x35, 0x00, 0x33, 0x33,
        0xAA, 0xBB, 0xCC, 0xDD,
        0x01, 0x02, 0x03, 0x04,
        0x11, 0x22, 0x33, 0x44
    };

    int failed = 0;
    for (size_t i = 0; i < sizeof(expected); ++i) {
        if (buffer[i] != expected[i]) {
            printf("Byte %2zu: got 0x%02X, expected 0x%02X\n", i, buffer[i], expected[i]);
            failed = 1;
        }
    }

    if (failed) {
        printf("=== Full dump ===\nExpected: ");
        for (size_t i = 0; i < sizeof(expected); ++i) printf("%02X ", expected[i]);
        printf("\nActual  : ");
        for (size_t i = 0; i < sizeof(expected); ++i) printf("%02X ", buffer[i]);
        printf("\n");
        assert(0 && "Header serialization mismatch");
    }

    return 0;
}