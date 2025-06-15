#include "cfdp.h"
#include <stdio.h>

int main() {
    cfdp_set_local_entity_id(1);
    cfdp_set_remote_entity_id(2);
    cfdp_set_output_socket("127.0.0.1", 3000);
    cfdp_send_all_files_in_directory("/tmp/cfdp_outbox");
    return 0;
}
