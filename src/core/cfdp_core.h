#ifndef CFDP_CORE_H
#define CFDP_CORE_H

// Standard libraries
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// CFDP components
#include "../mib/cfdp_mib.h"
#include "../fs/cfdp_fs.h"
#include "../comm/cfdp_comm.h"
#include "../ui/cfdp_ui.h"

#define CFDP_MAX_TRANSACTIONS 4 // Simulataneous transactions (static allocation)

// Transaction states
typedef enum {
    CFDP_TRAN_STATE_IDLE = 0,
    CFDP_TRAN_STATE_SENDING,
    CFDP_TRAN_STATE_RECEIVING,
    CFDP_TRAN_STATE_CANCELLED,
    CFDP_TRAN_STATE_COMPLETED
} CfdpTransactionState;

// Tansaction control block structure
typedef struct
{
    CfdpTransactionState state;
    uint32_t transaction_id;
    uint32_t dest_entity_id;

    // For sending: source file path
    // For receiving: dest file path
    const char *source_file;
    const char *dest_file;
    size_t bytes_sent;
    size_t bytes_received;
} CfdpTransaction;

// Initialize the CFDP core by setting up internal structures, return 0 on success
int cfdp_init(void);

// Process an incomming CFDP PDU, Called when a PDU is received by comm layer
void cfdp_process_pdu(const uint8_t *pdu, size_t pdu_len, uint32_t src_entity_id);

// (Internal) Start a new transaction to send a file (called by user API), returns a transaction ID or negative on error
int cfdp_start_transaction(uint32_t dest_id, const char *src_file, const char *dest_file, bool closure);

// Cancell an active transaction (by ID)
int cfdp_cancel_transaction(uint32_t transaction_id);

// Periodic tick to handle time_based events (e.g. timers), should be called periodically (e.g. once per second)
void cfdp_tick(void);

// Should do nothing more than just mark all transaction idle
void cfdp_shutdown(void);

#endif
