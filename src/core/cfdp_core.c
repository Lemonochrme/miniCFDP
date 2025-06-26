#include "cfdp_core.h"
#include <stdio.h>   // for printf
#include <string.h> 

#include "../ui/cfdp_ui.h"


// Static pool of transactions
static CfdpTransaction g_transactions[CFDP_MAX_TRANSACTIONS];
static uint32_t g_next_transaction_id = 1;  // simple transaction ID generator

// minimal logging macro
#ifdef CFDP_ENABLE_LOG
#define CFDP_LOG(fmt, ...)   printf("CFDP: " fmt "\n", __VA_ARGS__)
#else
#define CFDP_LOG(fmt, ...)   (void)0
#endif

int cfdp_init(void) {
    // initialize all transaction slots to idle
    memset(g_transactions, 0, sizeof(g_transactions));
    for (int i = 0; i < CFDP_MAX_TRANSACTIONS; i++) {
        g_transactions[i].state = CFDP_TRAN_STATE_IDLE;
    }
    g_next_transaction_id = 1;
    CFDP_LOG("CFDP core initialized (max transactions=%d)", CFDP_MAX_TRANSACTIONS);
    return 0;
}

static CfdpTransaction* allocate_transaction(void) {
    // find an idle transaction slot
    for (int i = 0; i < CFDP_MAX_TRANSACTIONS; ++i) {
        if (g_transactions[i].state == CFDP_TRAN_STATE_IDLE) {
            return &g_transactions[i];
        }
    }
    return NULL;
}

int cfdp_start_transaction(uint32_t dest_id, const char *src_file, const char *dest_file, bool closure) {
    CfdpTransaction *t = allocate_transaction();
    if (!t) {
        CFDP_LOG("No available transaction slots for new Put.request");
        return -1;  // no space
    }
    // Initialize transaction details
    t->state = CFDP_TRAN_STATE_SENDING;
    t->transaction_id = g_next_transaction_id++;
    t->dest_entity_id = dest_id;
    t->source_file = src_file;
    t->dest_file = dest_file;
    t->bytes_sent = 0;
    t->bytes_received = 0;
    CFDP_LOG("Starting transaction %u: send %s to entity %u as %s", 
             t->transaction_id, src_file, dest_id, dest_file);
    // ppen source file for reading
    int fd = cfdp_fs_open(src_file, false);
    if (fd < 0) {
        CFDP_LOG("Error: cannot open source file %s", src_file);
        t->state = CFDP_TRAN_STATE_CANCELLED;
        // notify user of transaction finished with failure
        cfdp_notify_event(t->transaction_id, CFDP_EVENT_TRANSACTION_FINISHED, CFDP_STATUS_FILE_ERROR);
        return -1;
    }
    // read file in segments and send PDUs
    uint8_t buffer[1024];
    size_t bytes_read;

    // TODO : send a Metadata PDU here with file size, name, etc... before sending file data.

    while ((bytes_read = cfdp_fs_read(fd, buffer, sizeof(buffer))) > 0) {
        // TODO : form a File Data PDU with offset and data
        cfdp_comm_send(dest_id, buffer, bytes_read);
        t->bytes_sent += bytes_read;
        // (Omitted: check if we need to throttle or wait for some condition)
    }
    cfdp_fs_close(fd);
    // TODO : Send an EOF PDU indicating end-of-file
    // TODO : if closure requested expect a finished PDU from receiver
    t->state = CFDP_TRAN_STATE_COMPLETED;
    CFDP_LOG("Transaction %u completed (sent %zu bytes)", t->transaction_id, t->bytes_sent);
    // notify user that transaction finished successfully
    cfdp_notify_event(t->transaction_id, CFDP_EVENT_TRANSACTION_FINISHED, CFDP_STATUS_SUCCESS);
    return t->transaction_id;
}

int cfdp_cancel_transaction(uint32_t transaction_id) {
    // find the transaction by ID
    for (int i = 0; i < CFDP_MAX_TRANSACTIONS; ++i) {
        if (g_transactions[i].transaction_id == transaction_id &&
            g_transactions[i].state != CFDP_TRAN_STATE_IDLE) {
            g_transactions[i].state = CFDP_TRAN_STATE_CANCELLED;
            CFDP_LOG("Transaction %u cancelled by user", transaction_id);
            
            // TODO : if Class 2 send Cancel PDU etc...
            
            cfdp_notify_event(transaction_id, CFDP_EVENT_TRANSACTION_FINISHED, CFDP_STATUS_CANCELLED);
            return 0;
        }
    }
    return -1; // transaction not found
}

void cfdp_process_pdu(const uint8_t *pdu, size_t pdu_len, uint32_t src_entity_id) {
    // TODO : Implement full parsing
    // Minimal PDU handling: In a complete implementation, we would parse the  header to determine PDU type (Metadata, File Data, EOF, Finished...) and act accordingly. Here we assume any received data is file data for an active receive transaction.
    CFDP_LOG("Received PDU of length %zu from entity %u", pdu_len, src_entity_id);
    // find or start a receive transaction for this source
    CfdpTransaction *rx = NULL;
    for (int i = 0; i < CFDP_MAX_TRANSACTIONS; ++i) {
        if (g_transactions[i].state != CFDP_TRAN_STATE_IDLE && 
            g_transactions[i].dest_entity_id == src_entity_id && 
            g_transactions[i].state != CFDP_TRAN_STATE_SENDING) {
            rx = &g_transactions[i];
            break;
        }
    }
    if (!rx) {
        // no existing transaction, start a new one for receiving
        rx = allocate_transaction();
        if (!rx) {
            CFDP_LOG("No slot available for incoming transaction from %u", src_entity_id);
            return;
        }
        rx->state = CFDP_TRAN_STATE_RECEIVING;
        rx->transaction_id = g_next_transaction_id++;
        rx->dest_entity_id = src_entity_id;
        rx->bytes_sent = 0;
        rx->bytes_received = 0;
        

        rx->dest_file = "recv_file.dat";
        CFDP_LOG("Initiated receive transaction %u from entity %u", rx->transaction_id, src_entity_id);

        int fd = cfdp_fs_open(rx->dest_file, true); // open file for writing
        if (fd < 0) {
            CFDP_LOG("Error: cannot create file %s for incoming data", rx->dest_file);
            rx->state = CFDP_TRAN_STATE_CANCELLED;
            cfdp_notify_event(rx->transaction_id, CFDP_EVENT_TRANSACTION_FINISHED, CFDP_STATUS_FILE_ERROR);
            return;
        }
        cfdp_fs_close(fd);
    }
    // write data to file (append mode)
    int fd = cfdp_fs_open(rx->dest_file, true);
    if (fd >= 0) {
        cfdp_fs_write(fd, pdu, pdu_len);
        cfdp_fs_close(fd);
    }
    rx->bytes_received += pdu_len;
    // TODO : here we assume this PDU could be final; in real logic, check if EOF PDU
    // for now assume this was EOF or last data.
    
    rx->state = CFDP_TRAN_STATE_COMPLETED;
    CFDP_LOG("Transaction %u: file reception complete (%zu bytes)", rx->transaction_id, rx->bytes_received);
    cfdp_notify_event(rx->transaction_id, CFDP_EVENT_TRANSACTION_FINISHED, CFDP_STATUS_SUCCESS);
}

void cfdp_tick(void) {
    // nothing for now
    // for Class 1 : check if any receive transaction have been incative for too long
}

void cfdp_shutdown(void) {
    // just mark all transactions idle
    for (int i = 0; i < CFDP_MAX_TRANSACTIONS; ++i) {
        g_transactions[i].state = CFDP_TRAN_STATE_IDLE;
    }
    CFDP_LOG("CFDP core shut down");
}