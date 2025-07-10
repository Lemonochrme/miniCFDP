#include "cfdp_core.h"
#include <stdio.h>   // for printf
#include <string.h>
#include <stdint.h>

#include "../ui/cfdp_user.h"
#include "../src/mib/cfdp_mib.h"
#include "cfdp_pdu.h"

// Todo : Check for MISRA compliance

// Static pool of transactions
static CfdpTransaction g_transactions[CFDP_MAX_TRANSACTIONS];
static uint32_t g_next_transaction_id = 1;  // simple transaction ID generator

// minimal logging macro
// Todo : Remove this as not MISRA compliant
#ifdef CFDP_ENABLE_LOG
#define CFDP_LOG(fmt, ...) printf("CFDP: " fmt "\n", ##__VA_ARGS__) 
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
        return -1;
    }

    t->state = CFDP_TRAN_STATE_SENDING;
    t->transaction_id = g_next_transaction_id++;
    t->dest_entity_id = dest_id;
    t->source_file = src_file;
    t->dest_file = dest_file;
    t->bytes_sent = 0;

    CFDP_LOG("Starting transaction %u: send %s to entity %u as %s",
             t->transaction_id, src_file, dest_id, dest_file);

    int fd = cfdp_fs_open(src_file, false);
    if (fd < 0) {
        CFDP_LOG("Error: cannot open source file %s", src_file);
        t->state = CFDP_TRAN_STATE_CANCELLED;
        cfdp_notify_event(t->transaction_id, CFDP_EVENT_TRANSACTION_FINISHED, CFDP_STATUS_FILE_ERROR);
        return -1;
    }

    uint32_t fsize = (uint32_t)cfdp_fs_size(fd); // forcibly truncated
    bool is_large = false; // disabled support
    (void)is_large;

    CfdpPduHeader hdr = {
        .version = CFDP_VERSION,
        .pdu_type = CFDP_PDU_TYPE_FILEDIR,
        .direction = CFDP_DIRECT_TOWARD_RECEIVER,
        .transmission_mode = CFDP_MODE_UNACKED,
        .crc_flag = 0,
        .large_flag_flag = 0,
        .segmentation_control = 0,
        .eid_length = 3,
        .seq_length = 3,
        .source_entity_id = cfdp_mib.local.entity_id,
        .transaction_seq_num = t->transaction_id,
        .dest_entity_id = dest_id
    };

    // --- Metadata PDU ---
    size_t meta_len;
    uint8_t meta_buf[512];

    hdr.data_field_length = CFDP_METADATA_PDU_OVERHEAD_LEN + strlen(src_file) + strlen(dest_file);
    meta_len = cfdp_build_metadata_pdu(meta_buf, &hdr, fsize, closure, src_file, dest_file);
    CFDP_LOG("helo metadata");
    cfdp_comm_send(dest_id, meta_buf, meta_len);

    // --- File Data PDUs ---
    uint8_t io_buf[1024];
    uint64_t offset = 0;

    size_t max_data_len = sizeof(io_buf) - CFDP_FILEDATA_HEADER_OVERHEAD;
    size_t read_len;
    while ((read_len = cfdp_fs_read(fd, io_buf + CFDP_FILEDATA_HEADER_OVERHEAD, max_data_len)) > 0) {
        CFDP_LOG("helo data");
        hdr.pdu_type = CFDP_PDU_TYPE_FILEDATA;
        hdr.data_field_length = CFDP_FILEDATA_CONTINUATION_LEN + CFDP_FILEDATA_OFFSET_LEN + read_len;
        size_t plen = cfdp_build_filedata_pdu(io_buf, &hdr, offset, io_buf + CFDP_FILEDATA_HEADER_OVERHEAD, read_len);
        cfdp_comm_send(dest_id, io_buf, plen);
        offset += read_len;
        t->bytes_sent += read_len;
    }

    cfdp_fs_close(fd);

    // --- EOF PDU ---
    uint8_t eof_buf[64];
    hdr.pdu_type = CFDP_PDU_TYPE_FILEDIR;
    hdr.data_field_length = CFDP_EOF_PDU_DATA_LEN;
    size_t eof_len = cfdp_build_eof_pdu(eof_buf, &hdr, 0x0, 0, fsize);
    CFDP_LOG("helo end of file");
    cfdp_comm_send(dest_id, eof_buf, eof_len);

    // --- Completion ---
    t->state = CFDP_TRAN_STATE_COMPLETED;
    CFDP_LOG("Transaction %u completed (sent %zu bytes)", t->transaction_id, t->bytes_sent);
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
        

        rx->dest_file = "recv_file.dat"; // the name is static for now for debug purposes
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

    // TODO : Implement PDU handling
    // here we assume this PDU could be final; in real logic, check if EOF PDU
    // for now assume this was EOF or last data.
    
    rx->state = CFDP_TRAN_STATE_COMPLETED;
    CFDP_LOG("Transaction %u: file reception complete (%zu bytes)", rx->transaction_id, rx->bytes_received);
    cfdp_notify_event(rx->transaction_id, CFDP_EVENT_TRANSACTION_FINISHED, CFDP_STATUS_SUCCESS);
}

void cfdp_tick(void) {
    // nothing for now
    // TODO : for Class 1 : check if any receive transaction have been incative for too long
}

void cfdp_shutdown(void) {
    // just mark all transactions idle
    for (int i = 0; i < CFDP_MAX_TRANSACTIONS; ++i) {
        g_transactions[i].state = CFDP_TRAN_STATE_IDLE;
    }
    CFDP_LOG("CFDP core shut down");
}