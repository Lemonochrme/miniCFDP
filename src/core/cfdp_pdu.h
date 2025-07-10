#ifndef CFDP_PDU_H
#define CFDP_PDU_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define CFDP_VERSION 0x1 // version 2 of the protocol
#define CFDP_PDU_TYPE_FILEDATA 0x1
#define CFDP_PDU_TYPE_FILEDIR 0x0

#define CFDP_DIRECT_TOWARD_RECEIVER 0x0
#define CFDP_DIRECT_TOWARD_SENDER 0x1 // for PDU forwarding

#define CFDP_MODE_ACKED 0x0
#define CFDP_MODE_UNACKED 0x1

#define CFDP_HEADER_LENGTH 4
#define CFDP_DIRECTIVE_METADATA 0x07
#define CFDP_DIRECTIVE_EOF 0x4

// support for only 32 bit IDs and 32 bit sequence numbers for simplicity
#define CFDP_ENTITY_ID_LENGTH 4
#define CFDP_TRANSFER_SEQUENCE_LENGTH 4

// ----==== Some Constants ====----

// Fixed PDU Header (always 16 bytes in this implementation)
#define CFDP_FIXED_HEADER_LEN             16

// Metadata PDU payload structure (excluding header)
#define CFDP_METADATA_DIRECTIVE_LEN       1   // 0x07
#define CFDP_METADATA_FLAGS_LEN           1   // closure + reserved
#define CFDP_METADATA_FILESIZE_LEN        4   // 32-bit only
#define CFDP_METADATA_SRCFN_LEN_LEN       1   // 1 byte length prefix
#define CFDP_METADATA_DSTFN_LEN_LEN       1   // idem
#define CFDP_METADATA_TLV_LIST_LEN        1   // optional, set to 0

#define CFDP_METADATA_PDU_OVERHEAD_LEN \
    (CFDP_METADATA_DIRECTIVE_LEN + CFDP_METADATA_FLAGS_LEN + CFDP_METADATA_FILESIZE_LEN + \
     CFDP_METADATA_SRCFN_LEN_LEN + CFDP_METADATA_DSTFN_LEN_LEN + CFDP_METADATA_TLV_LIST_LEN)

// File Data PDU
#define CFDP_FILEDATA_CONTINUATION_LEN    1   // always present
#define CFDP_FILEDATA_OFFSET_LEN          4   // always 32-bit (no large files)

#define CFDP_FILEDATA_HEADER_OVERHEAD \
    (CFDP_FIXED_HEADER_LEN + CFDP_FILEDATA_CONTINUATION_LEN + CFDP_FILEDATA_OFFSET_LEN)

// EOF PDU payload structure (excluding header)
#define CFDP_EOF_DIRECTIVE_LEN            1   // 0x04
#define CFDP_EOF_CONDITION_LEN            1   // 4-bit + 4-bit
#define CFDP_EOF_CHECKSUM_LEN             4   // CRC32
#define CFDP_EOF_FILESIZE_LEN             4   // 32-bit only

#define CFDP_EOF_PDU_DATA_LEN \
    (CFDP_EOF_DIRECTIVE_LEN + CFDP_EOF_CONDITION_LEN + CFDP_EOF_CHECKSUM_LEN + CFDP_EOF_FILESIZE_LEN)



// See 720x2g4
typedef struct {
    uint8_t version;                // 1 bit
    uint8_t pdu_type;               // 1 bit
    uint8_t direction;              // 1 bit
    uint8_t transmission_mode;      // 1 bit
    uint8_t crc_flag;               // 1 bit
    uint8_t large_flag_flag;        // 1 bit
    uint16_t data_field_length;     // 1 bit

    uint8_t segmentation_control; // 1 bit, always 0 in class 1
    uint8_t eid_length; // number of bytes minus 1 (0=1 byte, 3=4 bytes) Entity ID Length
    uint8_t seq_length; // same as above
    
    // uint8_t segment_metadata_flag; not used in class 1 - always 0
    uint32_t source_entity_id; // 4 bytes
    uint32_t transaction_seq_num; // 4 bytes
    uint32_t dest_entity_id; // 4 bytes
} CfdpPduHeader;

// serialize header into buffer, return the number of bytes written
size_t cfdp_serialize_header(uint8_t *buf, const CfdpPduHeader *hdr);

size_t cfdp_build_metadata_pdu(uint8_t *buf, const CfdpPduHeader *hdr, uint32_t file_size, uint8_t closure_requested, const char *source_filename, const char *dest_filename);

size_t cfdp_build_filedata_pdu(uint8_t *buf, const CfdpPduHeader *hdr, uint64_t offset, const uint8_t *data, size_t data_len);

size_t cfdp_build_eof_pdu(uint8_t *buf, const CfdpPduHeader *hdr, uint8_t condition_code, uint32_t checksum, uint64_t file_size);

#endif