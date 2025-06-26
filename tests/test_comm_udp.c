#include "../src/comm/cfdp_comm.h"
#include "../src/mib/cfdp_mib.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ENTITY_A_ID 1
#define ENTITY_B_ID 2

#define ENTITY_A_PORT 3001
#define ENTITY_B_PORT 3002

void setup_mib_for_entity(uint32_t local_id, uint16_t local_port, uint32_t remote_id, const char *remote_ip, uint16_t remote_port) {
    cfdp_mib.local.entity_id = local_id;
    cfdp_mib.remote_entity_count = 1;
    cfdp_mib.remote_entities[0].entity_id = remote_id;
    strcpy(cfdp_mib.remote_entities[0].addr, remote_ip);
    cfdp_mib.remote_entities[0].port = remote_port;
}

void test_comm_udp() {
    pid_t pid = fork();
    assert(pid >= 0);

    if (pid == 0) {
        // child process : receiver (entity B)
        setup_mib_for_entity(ENTITY_B_ID, ENTITY_B_PORT, ENTITY_A_ID, "127.0.0.1", ENTITY_A_PORT);
        assert(cfdp_comm_init(ENTITY_B_PORT) == 0);

        uint8_t buffer[256];
        uint32_t src_id = 0;

        int received = 0;
        for (int i = 0; i < 20; ++i) { // longer window
            received = cfdp_comm_receive(buffer, sizeof(buffer), &src_id);
            if (received > 0) break;
            usleep(100000); // 100ms
        }

        assert(received > 0);
        assert(strcmp((char*)buffer, "Hello CFDP") == 0);
        assert(src_id == ENTITY_A_ID);

        cfdp_comm_close();
        _exit(0);
    } else {
        // parent process : sender (entity A)
        setup_mib_for_entity(ENTITY_A_ID, ENTITY_A_PORT, ENTITY_B_ID, "127.0.0.1", ENTITY_B_PORT);
        assert(cfdp_comm_init(ENTITY_A_PORT) == 0);

        sleep(1); // donne au récepteur le temps de s’installer

        const char *msg = "Hello CFDP";
        assert(cfdp_comm_send(ENTITY_B_ID, (const uint8_t *)msg, strlen(msg) + 1) == 0);

        cfdp_comm_close();

        int status = 0;
        waitpid(pid, &status, 0);
        assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }
}

int main() {
    test_comm_udp();
    return 0;
}
