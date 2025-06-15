#include "cfdp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define CFDP_MAX_PACKET_SIZE 1024

static uint32_t local_id = 0;
static uint32_t remote_id = 0;

// UDP socket to simulate the link (easily portable to UART etc...)
static int sockfd = -1;
static struct sockaddr_in remote_addr; // Remote CFDP entity


// Metadata PDU, before data tranmission
// Contains file name, size and ID of the CFDP
static void send_metadata_pdu(const char* filename, size_t filesize) {
    uint8_t buffer[CFDP_MAX_PACKET_SIZE];
    size_t offset = 0;

    buffer[offset++] = 0x02; // version + type (directive) + direction
    buffer[offset++] = 0x10; // code of the Metadata PDU (fixed at 0x10 for testing pusposes)

    // Local + Remote CFDP IDs
    memcpy(buffer + offset, &local_id, 4); offset += 4;
    memcpy(buffer + offset, &remote_id, 4); offset += 4;

    uint16_t fname_len = strlen(filename);
    memcpy(buffer + offset, &fname_len, 2); offset += 2;
    memcpy(buffer + offset, filename, fname_len); offset += fname_len;
    memcpy(buffer + offset, &filesize, sizeof(filesize)); offset += sizeof(filesize);
    
    // Sending Metadata PDU via UDP for testing purposes
    sendto(sockfd, buffer, offset, 0, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
}

static void send_file_data_pdu(FILE* f) {
    uint8_t buffer[CFDP_MAX_PACKET_SIZE];
    size_t offset = 0;

    buffer[offset++] = 0x01;

    uint8_t data[CFDP_MAX_PACKET_SIZE - 1];
    size_t bytes;

    while ((bytes = fread(data, 1, sizeof(data), f)) > 0) {
        memcpy(buffer + 1, data, bytes);
        sendto(sockfd, buffer, bytes + 1, 0, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
    }
}

static void send_eof_pdu() {
    uint8_t buffer[8];
    buffer[0] = 0x02; // Directive type
    buffer[1] = 0x04; // Arbitrary EOF code
    sendto(sockfd, buffer, 2, 0, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
}

void cfdp_set_local_entity_id(uint32_t id) {
    local_id = id;
}

void cfdp_set_remote_entity_id(uint32_t id) {
    remote_id = id;
}

void cfdp_set_output_socket(const char* ip, uint16_t port) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &remote_addr.sin_addr);
}

void cfdp_send_all_files_in_directory(const char* directory) {
    DIR* dir = opendir(directory);
    if (!dir) return;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_REG) continue;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        FILE* f = fopen(path, "rb");
        if (!f) continue;
        struct stat st;
        stat(path, &st);
        send_metadata_pdu(entry->d_name, st.st_size);
        send_file_data_pdu(f);
        send_eof_pdu();
        fclose(f);
    }
    closedir(dir);
}