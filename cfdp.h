#ifndef CFDP_H
#define CFDP_H

#include <stdint.h>

void cfdp_set_local_entity_id(uint32_t id);
void cfdp_set_remote_entity_id(uint32_t id);
void cfdp_set_output_socket(const char* ip, uint16_t port);
void cfdp_send_all_files_in_directory(const char* directory);

#endif