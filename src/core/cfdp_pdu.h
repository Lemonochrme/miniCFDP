#ifndef CFDP_PDU_H
#define CFDP_PDU_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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


typedef struct {
    uint8_t version;
    uint8_t pdu_type;
    uint8_t direction;
    uint8_t transmission_mode;
    uint8_t crc_flag;
    uint8_t large_flag_flag;
    uint16_t data_field_length;

    uint8_t segmentation_control;
    uint8_t id_length;
    // uint8_t segment_metadata_flag; not used in class 1 - always 0
    uint8_t seq_length;

    uint32_t source_entity_id;
    uint32_t transaction_seq_num;
    uint32_t dest_entity_id;
} CfdpPduHeader;

#endif