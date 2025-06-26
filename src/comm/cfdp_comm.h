#ifndef CFDP_COMM_H
#define CFDP_COMM_H

#include <stddef.h>
#include <stdint.h>


// Initialize the communication layer
int cfdp_comm_init(uint16_t local_port);

// Send a CFDP PDU to a destination CFDP entity, the dest_entity_id will be mapped to a network address via the MIB
int cfdp_comm_send(uint32_t dest_entity_id, const uint8_t *data, size_t len);

// receive a CFDP PDU (non-blocking)
int cfdp_comm_receive(uint8_t *buffer, size_t buf_len, uint32_t *out_src_entity_id);

// shutdown the communication layer (close sockets, etc...)
int cfdp_comm_close(void);

#endif