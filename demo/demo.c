#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "../src/core/cfdp_core.h"
#include "../src/comm/cfdp_comm.h"
#include "../src/ui/cfdp_user.h"
#include "../src/mib/cfdp_mib.h"

static void run_receiver(void) {
    // receiving instance : entity_id=2, port=3001
    cfdp_mib.local.entity_id = 2;
    cfdp_init();
    if (cfdp_comm_init(3001) < 0) {
        perror("Receiver: cfdp_comm_init");
        exit(1);
    }
    printf("[2] Waiting for data...\n");
    // receiving loop
    while(1) {
        uint8_t buf[1500];
        uint32_t src;
        int len = cfdp_comm_receive(buf, sizeof(buf), &src);
        if (len > 0) {
            cfdp_process_pdu(buf, len, src);
        }
        cfdp_tick();
        usleep(100000);
    }
}

static void run_sender(const char *file) {
    cfdp_mib.local.entity_id = 1; // entity_id=1, port=3000
    cfdp_init();
    if (cfdp_comm_init(3000) < 0) {
        perror("Sender: cfdp_comm_init");
        exit(1);
    }
    sleep(1); // give time to receiver to start
    printf("[1] Sending '%s' to entity 2...\n", file);
    int tid = cfdp_request_put(2, file, "received_image.png", false);
    if (tid < 0) {
        fprintf(stderr, "[1] Error starting transaction\n");
        exit(1);
    }
    printf("[1] Transaction %d started\n", tid);
    
    sleep(3); // wait until transfer done
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <image_to_send.png>\n", argv[0]);
        return 1;
    }
    const char *img = argv[1];

    /* Expecting correct configuration in src/mib/cfdp_mib.c (two remote entities because we are using only one mib for simplicity sake):
         .remote_entities = {
           { .entity_id = 1, .addr = "127.0.0.1", .port = 3000 },
           { .entity_id = 2, .addr = "127.0.0.1", .port = 3001 }
         },
         .remote_entity_count = 2
    */

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        run_receiver(); // children process = receiver
    } else {
        run_sender(img); // parent process = emitter

        kill(pid, SIGTERM); // kill the receiver
        wait(NULL);
        printf("[1] Demo done.\n");
    }
    return 0;
}
