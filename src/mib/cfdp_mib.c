#include "cfdp_mib.h"

// TODO : better configuration process through a function and configuration files

CfdpMib cfdp_mib = {
    .local = {
        .entity_id = 1,
        .default_closure_requested = false,
        .crc_required = false,
        .ack_timer_sec = 10,
        .ack_limit = 4,
        .nack_timer_sec = 10,
        .nack_limit = 4,
        .inactivity_timer_sec = 42, // hehe
        .max_file_segment_len = 1024
    },
    .remote_entities = {
        // example entry
        { .entity_id = 2, .addr = "127.0.0.1", .port = 3000 }
    },
    .remote_entity_count = 1
};