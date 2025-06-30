#include "cfdp_user.h"

#include <stdio.h>
#include "../core/cfdp_core.h"

// static pointer to user-provided callback function
static CfdpIndicationCallback s_indication_cb = NULL;

void cfdp_register_callback(CfdpIndicationCallback cb) {
    s_indication_cb = cb;
}

// this function is called by CFDP core (or by UI) to dispatch an event to the user
void cfdp_notify_event(uint32_t transaction_id, CfdpEvent event, CfdpStatus status) {
    if (s_indication_cb) {
        s_indication_cb(transaction_id, event, status);
    } else {
        // TODO : (do somthing) if no callback defined do nothing for now
    }
}

int cfdp_request_put(uint32_t dest_entity_id, const char *source_file, const char *dest_file, bool closure_requested) {
    return cfdp_start_transaction(dest_entity_id, source_file, dest_file, closure_requested);
}

int cfdp_request_cancel(uint32_t transaction_id) {
    return cfdp_cancel_transaction(transaction_id);
}