#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "../src/core/cfdp_pdu.h"

int main(void) {
    CfdpPduHeader hdr = {
        .version = CFDP_VERSION,
        .pdu_type = CFDP_PDU_TYPE_FILEDIR,
        .direction = CFDP_DIRECT_TOWARD_RECEIVER,
        .transmission_mode = CFDP_MODE_UNACKED,
        .crc_flag = 0,
        .large_flag_flag = 0,
        .data_field_length = 10,
        .segmentation_control = 0,
        .eid_length = 3,
        .seq_length = 3,
        .source_entity_id = 0xAABBCCDD,
        .transaction_seq_num = 0x01020304,
        .dest_entity_id = 0x11223344
    };

    uint8_t buffer[64];
    size_t len = cfdp_build_eof_pdu(buffer, &hdr, 0, 0xDEADBEEF, 42);

    // Fixed PDU Header
    // 0x24, 0x00, 0x1B, 0x33,  : Protocol version 2 (0b001), File Directive (0), unack (0), no crc (0), large=0, dfl=27, seg ctrl=0, len=4
    // 0xAA, 0xBB, 0xCC, 0xDD,  : Source Entity ID
    // 0x01, 0x02, 0x03, 0x04,  : Transaction Seq Num
    // 0x11, 0x22, 0x33, 0x44   : Dest Entity ID

    // End-Of-File Payload
    // 0x04,       : EOF directive (0x04)
    // 0x00,       : condition code 0, spare 0
    // 0xDE, 0xAD, 0xBE, 0xEF,  : checksum
    // 0x00, 0x00, 0x00, 0x2A   : file size = 42 bytes

    uint8_t expected[] = {
        0x24, 0x00, 0x0A, 0x33,
        0xAA, 0xBB, 0xCC, 0xDD,
        0x01, 0x02, 0x03, 0x04,
        0x11, 0x22, 0x33, 0x44,
        0x04, 0x00,
        0xDE, 0xAD, 0xBE, 0xEF,
        0x00, 0x00, 0x00, 0x2A
    };

    assert(len == sizeof(expected));
    for (size_t i = 0; i < len; ++i)
        assert(buffer[i] == expected[i]);

    return 0;
}
