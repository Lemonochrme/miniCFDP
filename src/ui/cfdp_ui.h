#ifndef CFDP_USER_H
#define CFDP_USER_H

#include <stdint.h>
#include <stdbool.h>

// event types for CFDP indications
typedef enum {
    CFDP_EVENT_TRANSACTION_FINISHED = 1,
    CFDP_EVENT_FILE_SEGMENT_RECEIVED = 2,
} CfdpEvent;

// status/condition codes for transaction completion
typedef enum {
    CFDP_STATUS_SUCCESS = 0,
    CFDP_STATUS_CANCELLED,
    CFDP_STATUS_FILE_ERROR,
    CFDP_STATUS_ABORTED
} CfdpStatus;


// callback type for CFDP indications
typedef void (*CfdpIndicationCallback)(uint32_t transaction_id, CfdpEvent event, CfdpStatus status);


// notification function
void cfdp_notify_event(uint32_t transaction_id, CfdpEvent event, CfdpStatus status);


// register a callback for CFDP events (override any existing)
void cfdp_register_callback(CfdpIndicationCallback cb);


// ----==== CFDP Service Primitives (Requests) ====----

// Initiate a file transfert (Put.request)
int cfdp_request_put(uint32_t dest_entity_id, const char *source_file, const char *dest_file, bool closure_requested);

// Cancel an ongoing file transfer (Cancel.request)
int cfdp_request_cancel(uint32_t transaction_id);

// TODO : Other primitives

#endif