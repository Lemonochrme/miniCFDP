#include "cfdp_mib.h"

// TODO : better configuration process through une fonction ou un fichier de conf

CfdpMib cfdp_mib = {
    .local = {
        .entity_id               = 1,
        .default_closure_requested = false,
        .crc_required            = false,
        .ack_timer_sec           = 10,
        .ack_limit               = 4,
        .nack_timer_sec          = 10,
        .nack_limit              = 4,
        .inactivity_timer_sec    = 42,
        .max_file_segment_len    = 1024
    },
    .remote_entities = {
        // entité 1 ↔ port 3000
        { .entity_id = 1, .addr = "127.0.0.1", .port = 3000 },
        // entité 2 ↔ port 3001
        { .entity_id = 2, .addr = "127.0.0.1", .port = 3001 }
    },
    // on a maintenant deux cibles possibles
    .remote_entity_count = 2
};
