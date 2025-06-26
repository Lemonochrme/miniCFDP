#ifndef CFDP_MIB_H
#define CFDP_MIB_H

#include <stdint.h>
#include <stdbool.h>

// maximum number of known remote CFDP entities
#define CFDP_MAX_REMOTE 4

typedef struct {
    uint32_t entity_id; // CFDP entity ID of the remote
    char addr[64]; // address string (IP adrress)
    uint16_t port; // port number 
} CfdpRemoteEntity;

// structure for local entity configuration
typedef struct {
    uint32_t entity_id; // local CFDP entity ID
    bool default_closure_requested; // default closure request flag for Class 1
    bool crc_required; // whether to use CRC on PDUs
    uint32_t ack_timer_sec; // ACK timeout (for Class 2) in seconds
    uint8_t ack_limit; // ACK retry limit (for Class 2)
    uint32_t nack_timer_sec; // For Class 2
    uint8_t nack_limit; // For Class 2
    uint32_t inactivity_timer_sec; // Inactivity timeout for transactions
    uint32_t max_file_segment_len; 
} CfdpLocalEntityMib;


// MIB top level structure
typedef struct {
    CfdpLocalEntityMib local;
    CfdpRemoteEntity remote_entities[CFDP_MAX_REMOTE];
    uint32_t remote_entity_count;
} CfdpMib;

// global MIB instance
extern CfdpMib cfdp_mib;

// TODO : Function to initialise MIB from defaults or config file

#endif