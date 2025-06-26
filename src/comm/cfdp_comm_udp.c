#include "cfdp_comm.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../mib/cfdp_mib.h"

static int g_udp_socket = -1;

int cfdp_comm_init(uint16_t local_port) {
    // create UDP socket
    g_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_udp_socket < 0) {
        perror("socket");
        return -1;
    }
    // cind to the specified local port on all interfaces
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(local_port);
    if (bind(g_udp_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(g_udp_socket);
        g_udp_socket = -1;
        return -1;
    }

    return 0;
}

int cfdp_comm_send(uint32_t dest_entity_id, const uint8_t *data, size_t len) {
    if (g_udp_socket < 0) return -1;
    // find remote entity in MIB to get address and port
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = 0;
    dest_addr.sin_addr.s_addr = 0;
    for (unsigned i = 0; i < cfdp_mib.remote_entity_count; i++) {
        if (cfdp_mib.remote_entities[i].entity_id == dest_entity_id) {
            dest_addr.sin_port = htons(cfdp_mib.remote_entities[i].port);
            dest_addr.sin_addr.s_addr = inet_addr(cfdp_mib.remote_entities[i].addr);
            break;
        }
    }
    if (dest_addr.sin_port == 0) {
        fprintf(stderr, "CFDP: Unknown destination entity %u\n", dest_entity_id);
        return -1;
    }
    ssize_t result = sendto(g_udp_socket, data, len, 0,
                             (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    return (result == (ssize_t)len) ? 0 : -1;
}

int cfdp_comm_receive(uint8_t *buffer, size_t buf_len, uint32_t *out_src_entity_id) {
    if (g_udp_socket < 0) return -1;
    struct sockaddr_in src_addr;
    socklen_t addrlen = sizeof(src_addr);
    // use recvfrom with MSG_DONTWAIT for non-blocking behavior
    ssize_t recvd = recvfrom(g_udp_socket, buffer, buf_len, MSG_DONTWAIT,
                              (struct sockaddr*)&src_addr, &addrlen);
    if (recvd < 0) {
        return 0; // no data or error (treat as no data for simplicity)
    }
    // determine the source entity ID from MIB using the source IP:port
    uint32_t src_entity = 0;
    char ip_str[16];
    inet_ntop(AF_INET, &src_addr.sin_addr, ip_str, sizeof(ip_str));
    uint16_t src_port = ntohs(src_addr.sin_port);
    for (unsigned i = 0; i < cfdp_mib.remote_entity_count; i++) {
        if (strcmp(cfdp_mib.remote_entities[i].addr, ip_str) == 0 &&
            cfdp_mib.remote_entities[i].port == src_port) {
            src_entity = cfdp_mib.remote_entities[i].entity_id;
            break;
        }
    }
    if (out_src_entity_id) *out_src_entity_id = src_entity;
    return (int)recvd;
}

int cfdp_comm_close(void) {
    if (g_udp_socket >= 0) {
        close(g_udp_socket);
        g_udp_socket = -1;
    }
    return 0;
}