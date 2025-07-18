#include "cfdp_pdu.h"

// utility functions

// write big endian integers 32 bits
static void write_be32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)(v >> 0);
}
// write big endian integers 64 bits
static void write_be64(uint8_t *p, uint64_t v) {
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
    second |= (hdr->segmentation_control & 0x1) << 7;     // x000 0000
    second |= (hdr->eid_length & 0x07) << 4;              // 0xxx 0000
    second |= (0 << 3); // segment metadata always 0 for now 0000 x000
    second |= (hdr->seq_length & 0x07) << 0;              // 0000 0xxx

    buf[0] = first;
    buf[1] = (uint8_t)(hdr->data_field_length >> 8); // the first 2 bytes
    buf[2] = (uint8_t)(hdr->data_field_length & 0xFF); // the last 2 bytes 
    buf[3] = second;

    write_be32(buf + 4, hdr->source_entity_id);
    write_be32(buf + 8, hdr->transaction_seq_num);
    write_be32(buf + 12, hdr->dest_entity_id);

    return 16;
}

size_t cfdp_build_metadata_pdu(uint8_t *buf, const CfdpPduHeader *hdr, uint32_t file_size, uint8_t closure_requested, const char *source_filename, const char *dest_filename) {
    
    // buf[pos++]; -> buf[pos]; pos++;

    size_t pos = 0; // offset "selector"

    // serialize fixed header (16 bytes)
    pos += cfdp_serialize_header(buf + pos, hdr);

    // file directive code
    buf[pos++] = CFDP_DIRECTIVE_METADATA;

    // 0x40 = 0100 0000 closure requested + 6 reserved bytes = 0 
    buf[pos++] = (closure_requested ? 0x40 : 0x00);

    // file size (4 bytes big endian)
    write_be32(buf + pos, file_size);
    pos += 4;

    // source filename Length-Value
    uint8_t len_src = (uint8_t)strlen(source_filename);
    buf[pos++] = len_src;
    memcpy(buf + pos, source_filename, len_src);
    pos += len_src;

    // destination filename Length-Value
    uint8_t len_dst = (uint8_t)strlen(dest_filename);
    buf[pos++] = len_dst;
    memcpy(buf + pos, dest_filename, len_dst);
    pos += len_dst;

    // no Type-Length-Value option = 0x00
    buf[pos++] = 0x00;

    return pos;
}



size_t cfdp_build_filedata_pdu(uint8_t *buf, const CfdpPduHeader *hdr, uint64_t offset, const uint8_t *data, size_t data_len) {
    size_t pos = 0;

    // serialize header (16 bytes)
    pos += cfdp_serialize_header(buf + pos, hdr);

    // no segment data, no record continuation (always 0x00)
    buf[pos++] = 0x00;

    // segment offset, size depends on large_flag_size
    if (hdr->large_flag_flag) {
        write_be64(buf + pos, offset);
        pos += 8;
    } else {
        write_be32(buf + pos, (uint32_t)offset);
        pos += 4;
    }

    memcpy(buf + pos, data, data_len);
    pos += data_len;

    return pos;
}







size_t cfdp_build_eof_pdu(uint8_t *buf, const CfdpPduHeader *hdr, uint8_t condition_code, uint32_t checksum, uint64_t file_size) {
    size_t pos = 0;

    pos += cfdp_serialize_header(buf + pos, hdr);

    // direction code : 0x04 = EOF
    buf[pos++] = CFDP_DIRECTIVE_EOF;

    // condition code (4 bits) + spare 0
    buf[pos++] = (condition_code & 0x0F) << 4;

    // 32 bits checksum
    write_be32(buf + pos, checksum);
    pos += 4;

    // file size (FSS : 32 or 64 bits)
    if (hdr->large_flag_flag) {
        write_be64(buf + pos, file_size);
        pos += 8;
    } else {
        write_be32(buf + pos, (uint32_t)file_size);
        pos += 4;
    }

    return pos;
}