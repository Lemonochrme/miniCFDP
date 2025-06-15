#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAXBUF 2048

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3000);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));

    FILE* current_file = NULL;
    char current_filename[256] = "received_file.tmp";

    mkdir("/tmp/cfdp_received", 0777);

    uint8_t buffer[MAXBUF];
    while (1) {
        ssize_t len = recv(sockfd, buffer, sizeof(buffer), 0);
        if (len <= 0) continue;
        uint8_t type = buffer[0];
        if (type == 0x02 && buffer[1] == 0x10) {
            uint16_t fname_len;
            memcpy(&fname_len, buffer + 10, 2);
            memcpy(current_filename, buffer + 12, fname_len);
            current_filename[fname_len] = '\0';
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "/tmp/cfdp_received/%s", current_filename);
            if (current_file) fclose(current_file);
            current_file = fopen(fullpath, "wb");
        } else if (type == 0x01 && current_file) {
            fwrite(buffer + 1, 1, len - 1, current_file);
        } else if (type == 0x02 && buffer[1] == 0x04) {
            if (current_file) { fclose(current_file); current_file = NULL; }
        }
    }

    return 0;
}