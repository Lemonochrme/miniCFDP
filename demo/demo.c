#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdint.h>

#include "../src/core/cfdp_core.h"
#include "../src/comm/cfdp_comm.h"
#include "../src/ui/cfdp_user.h"
#include "../src/mib/cfdp_mib.h"

// --- DEMO CALLBACK ---
// Cette fonction est appelée par le moteur CFDP pour chaque événement (fin de transaction, etc.)
static void demo_callback(uint32_t transaction_id, CfdpEvent event, CfdpStatus status) {
    printf("[DEMO] Transaction %u - Event %d - Status %d\n", transaction_id, event, status);
}

// --- RECEIVER (Entity 2) ---
static void run_receiver(void) {
    printf("[2] Initialising Receiver (CFDP Entity 2)...\n");

    // Configuration locale du MIB
    cfdp_mib.local.entity_id = 2;

    // Enregistrement du callback de trace
    cfdp_register_callback(demo_callback);

    // Initialisation du cœur CFDP
    cfdp_init();

    // Initialisation UDP sur le port de réception (3001)
    if (cfdp_comm_init(3001) < 0) {
        perror("[2] cfdp_comm_init");
        exit(EXIT_FAILURE);
    }

    printf("[2] Receiver ready. Waiting for incoming PDUs on port 3001...\n");

    // Boucle principale de réception CFDP
    while (1) {
        uint8_t buf[1500];
        uint32_t src_entity = 0;

        // Lecture non bloquante d’un PDU CFDP reçu (via UDP)
        int len = cfdp_comm_receive(buf, sizeof(buf), &src_entity);
        if (len > 0) {
            printf("[2] Received PDU (%d bytes) from entity %u\n", len, src_entity);
            cfdp_process_pdu(buf, len, src_entity);
        }

        // Tick périodique pour la gestion des timers CFDP (inactivité, retransmission si Class 2)
        cfdp_tick();

        usleep(100000);  // 100 ms
    }
}

// --- SENDER (Entity 1) ---
static void run_sender(const char *file) {
    printf("[1] Initialising Sender (CFDP Entity 1)...\n");

    // Configuration locale du MIB
    cfdp_mib.local.entity_id = 1;

    // Enregistrement du callback pour recevoir les notifications
    cfdp_register_callback(demo_callback);

    // Initialisation du cœur CFDP
    cfdp_init();

    // Initialisation UDP sur le port d’émission (3000)
    if (cfdp_comm_init(3000) < 0) {
        perror("[1] cfdp_comm_init");
        exit(EXIT_FAILURE);
    }

    sleep(1);  // Délai pour laisser le récepteur s’initialiser

    printf("[1] Starting file transfer of '%s' to CFDP entity 2...\n", file);

    // Lancement d’une transaction CFDP Class 1 vers l’entité 2 (pas de "closure" demandée ici)
    int tid = cfdp_request_put(2, file, "received_image.png", false);
    if (tid < 0) {
        fprintf(stderr, "[1] Failed to start transaction.\n");
        exit(EXIT_FAILURE);
    }

    printf("[1] Transaction %d started.\n", tid);

    sleep(3);  // Laisser le temps à la transaction de se terminer (à adapter selon taille fichier)
}

// --- MAIN DEMO ENTRY POINT ---
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_to_send.png>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *img = argv[1];

    /*
        PRÉREQUIS MIB :
        Dans src/mib/cfdp_mib.c, s'assurer que les deux entités sont configurées comme suit :
            .remote_entities = {
                { .entity_id = 1, .addr = "127.0.0.1", .port = 3000 },
                { .entity_id = 2, .addr = "127.0.0.1", .port = 3001 }
            },
            .remote_entity_count = 2
        Cette table permet au module cfdp_comm_udp.c de faire la résolution d’adresse
        à partir de l’ID d’entité CFDP (vers IP+port UDP).
    */

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Processus enfant = Récepteur
        run_receiver();
    } else {
        // Processus parent = Émetteur
        run_sender(img);

        // Terminer proprement le récepteur une fois la démo finie
        kill(pid, SIGTERM);
        wait(NULL);
        printf("[1] Demo finished. Receiver shut down.\n");
    }

    return 0;
}
