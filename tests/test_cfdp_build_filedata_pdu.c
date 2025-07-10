#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "../src/core/cfdp_pdu.h"

int main(void) {
    CfdpPduHeader hdr = {
        .version = CFDP_VERSION,
        .pdu_type = CFDP_PDU_TYPE_FILEDATA,
        .direction = CFDP_DIRECT_TOWARD_RECEIVER,
        .transmission_mode = CFDP_MODE_UNACKED,
        .crc_flag = 0,
        .large_flag_flag = 0,
        .data_field_length = 1 + 4 + 4,
        .segmentation_control = 0,
        .eid_length = 3,
        .seq_length = 3,
        .source_entity_id = 0xAABBCCDD,
        .transaction_seq_num = 0x01020304,
        .dest_entity_id = 0x11223344
    };

    uint8_t buffer[64];
    uint8_t data[] = {'D','A','T','A'};
    size_t len = cfdp_build_filedata_pdu(buffer, &hdr, 16, data, 4);

    // Fixed PDU Header
    // 0x34, 0x00, 0x1B, 0x33,  : Protocol version 2 (0b001), File Data (1), unack (0), no crc (0), large=0, dfl=27, seg ctrl=0, len=4
    // 0xAA, 0xBB, 0xCC, 0xDD,  : Source Entity ID
    // 0x01, 0x02, 0x03, 0x04,  : Transaction Seq Num
    // 0x11, 0x22, 0x33, 0x44   : Dest Entity ID

    // Filedata Payload
    // 0x00, : record continuation + segment metadata len
    // 0x00, 0x00, 0x00, 0x10, : offset 16 (large_flag = 0, 32-bit offset)
    // "DATA"

    uint8_t expected[] = {
        0x34, 0x00, 0x09, 0x33,
        0xAA, 0xBB, 0xCC, 0xDD,
        0x01, 0x02, 0x03, 0x04,
        0x11, 0x22, 0x33, 0x44,
        0x00,                   
        0x00, 0x00, 0x00, 0x10, 
        'D','A','T','A'
    };

    assert(len == sizeof(expected));
    for (size_t i = 0; i < len; ++i) {
        if (buffer[i] != expected[i]) {
            printf("Mismatch at byte %zu: got 0x%02X, expected 0x%02X\n", i, buffer[i], expected[i]);
        }
        assert(buffer[i] == expected[i]);
    }

    return 0;
}
