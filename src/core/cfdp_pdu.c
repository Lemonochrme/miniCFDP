#include "cfdp_pdu.h"

// utility functions

// write big endian integers 32 bit
static void write_be32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)(v >> 0);
}

// write big endian integers 64 bit
static void write_be64(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 56);
    p[1] = (uint8_t)(v >> 48);
    p[2] = (uint8_t)(v >> 40);
    p[3] = (uint8_t)(v >> 32);
    p[4] = (uint8_t)(v >> 24);
    p[5] = (uint8_t)(v >> 16);
    p[6] = (uint8_t)(v >> 8);
    p[7] = (uint8_t)(v >> 0);
}

// Header serialization
size_t cfdp_serialize_header(uint8_t *buf, const CfdpPduHeader *hdr) {
    uint8_t first = 0;
    first |= (CFDP_VERSION & 0x07) << 5;            // xxx0 0000
    first |= (hdr->pdu_type & 0x01) << 4;           // 000x 0000
    first |= (hdr->direction & 0x1) << 3;           // 0000 x000
    first |= (hdr->transmission_mode & 0x1) << 2;   // 0000 0x00
    first |= (hdr->crc_flag & 0x1) << 1;            // 0000 00x0
    first |= (hdr->large_flag_flag & 0x1) << 0;     // 0000 000x

    uint8_t second = 0;
    second |= (hdr->segmentation_control & 0x1) << 7; // x000 0000
    second |= (hdr->eid_length & 0x07) << 4;          // 0xxx 0000
    second |= (0 << 3); // segment metadata always 0 for now 0000 x000
    second |= (hdr->seq_length & 0x07) << 0;          // 0000 000x

    buf[0] = first;
    buf[1] = (uint8_t)(hdr->data_field_length >> 8); // the first 2 bytes
    buf[2] = (uint8_t)(hdr->data_field_length & 0xFF); // the last 2 bytes 
    buf[3] = second;

    write_be32(buf + 4, hdr->source_entity_id);
    write_be32(buf + 8, hdr->transaction_seq_num);
    write_be32(buf + 12, hdr->dest_entity_id);
}

