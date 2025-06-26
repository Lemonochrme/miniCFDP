#include "cfdp_core.h"
#include <stdio.h>
#include <string.h>

// static pool of transactions
static CfdpTransaction g_transaction[CFDP_MAX_TRANSACTIONS];
static uint32_t g_next_transaction_id = 1; // SImple transaction id generator


// Minimal logging macro for debug
#ifdef CFDP_ENABLE_LOG
#define CFDP_LOG(fmt, ...) printf("CFDP: " fmt "\n", __VA_ARGS__)
#else
#define CFDP_LOG(fmt, ...) (void)0
#endif

int cfdp_init(void) {
    // init all transaction slots to idle
    memset(g_transaction, 0, sizeof(g_transaction));

    for (int i = 0; i < CFDP_MAX_TRANSACTIONS; i++) {
        g_transaction[i].state = CFDP_TRAN_STATE_IDLE;
    }

    g_next_transaction_id = 1;

    CFDP_LOG("Initialized CFDP core (max transactions=%d)", CFDP_MAX_TRANSACTIONS);
    return 0;
}

static CfdpTransaction* allocate_transaction(void) {
    // find an idle transaction slot
    for (int i = 0; i < CFDP_MAX_TRANSACTIONS; i++) {
        if (g_transaction[i].state == CFDP_TRAN_STATE_IDLE) {
            return &g_transaction[i];
        }
    }
    return NULL;
}

int cfdp_start_transaction(uint32_t dest_id, const char *src_file, const char *dest_file, bool closure) {
    CfdpTransaction *t = allocate_transaction();

    if (!t) {
        return -1;
    }

    // Initialize transaction details
    t->state = CFDP_TRAN_STATE_SENDING;
    t->transaction_id = g_next_transaction_id++;
    t->dest_entity_id = dest_id;
    t->source_file = src_file;
    t->dest_file = dest_file;
    t->bytes_sent = 0;
    t->bytes_received = 0;

    // Open source file for reading
    // TODO : call to cfdp_fs ...

    // Read file in segments and send PDUs
    uint8_t buffer[1024];
    size_t bytes_read;

    // Send a metadata PDU here with file size name etc... before sending file data

    // Send a end of file EOF PDU here after sending the file

    // Notify user
}

int cfdp_cancel_transaction(utin32_t transaction_id) {
    // Find the transaction by ID
    for (int i = 0; i < CFDP_MAX_TRANSACTIONS; i++) {
        if (g_transaction[i].transaction_id == transaction_id && g_transaction[i].state != CFDP_TRAN_STATE_IDLE) {
            g_transaction[i].state = CFDP_TRAN_STATE_CANCELLED;

            // Cfdp notify event

            return 0;
        }
    }
    return -1; // Transaction not found
}

// Minimal PDU handling, TODO : in a complete implementation we must parse the header to determine the PDU type : Metadata, File Data, EOF...
// Here, for now, we assume any received data is file data
void cfdp_process_pdu(const uint8_t *pdu, size_t pdu_len, utint32_t src_entity_id) {
    // Find or start a transaction for this source
    CfdpTransaction *rx = NULL;

    for (int i = 0; i < CFDP_MAX_TRANSACTIONS; i++) {
        if (g_transaction[i].state != CFDP_TRAN_STATE_IDLE && g_transaction[i].dest_entity_id == src_entity_id && g_transaction[i].state != CFDP_TRAN_STATE_SENDING) {
            rx = &g_transaction[i];
            break;
        }
    }

    if (!rx) {
        // No existing transaction, start a new one for receiving
        rx = allocate_transaction();
        if (!rx) {
            // no slot available
            return;
        }

        rx->state = CFDP_TRAN_STATE_RECEIVING;
        rx->transaction_id = g_next_transaction_id++;
        rx->dest_entity_id = src_entity_id;
        rx->bytes_sent = 0;

        rx->dest_file = "recv_file.dat";

        // TODO : file system calls
    }
}

void cfdp_shutdown(void) {
    for (int i = 0; i < CFDP_MAX_TRANSACTIONS; i++) {
        g_transaction[i].state = CFDP_TRAN_STATE_IDLE;
    }
}