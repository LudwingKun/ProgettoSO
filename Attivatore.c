#include "lib/GeneralUtils.h"


struct Atomi *pids_ptr;
struct Dump *dump_ptr;


int main(int argc, char *argv[]) {
    int msgid, shmid, shmidReport,i=0;
    message myMessage;
    int shmidInit, STEP_ATTIVATORE;
    int *shmidInit_ptr;
    int num_bytes;
    int semid;
    struct sigaction sa;
    sa.sa_handler = gestore_segnale;
    sigaction(SIGTERM, &sa, NULL);

    printf("HELLO DA ATTIVATORE.... \n");
    if (argc != 2) {
        fprintf(stderr, "Error, wrong number of argument for atomo\n");
        exit(EXIT_FAILURE);
    }

    shmidInit = atoi(argv[1]);
    /*Collega la memoria condivisa Init al puntatore*/
    shmidInit_ptr = (int *) shmat(shmidInit, NULL, 0);
    if ((int *) shmidInit_ptr == (void *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }

    STEP_ATTIVATORE = shmidInit_ptr[3];
    semid = shmidInit_ptr[10];
    shmidReport = shmidInit_ptr[9];
    shmid = shmidInit_ptr[8];
    msgid = shmidInit_ptr[11];

/*
    printf("STEP_ATTIVATORE: %d\n", STEP_ATTIVATORE);
    printf("semid: %d\n", semid);
    printf("shmidReport: %d\n", shmidReport);
    printf("shmid: %d\n", shmid);
    printf("msgid: %d\n", msgid);
*/


    /* detach memoria condivisa per init */
    if (shmdt(shmidInit_ptr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }



    /*Collega la memoria condivisa Report al puntatore*/
    dump_ptr = (struct Dump *) shmat(shmidReport, NULL, 0);
    if ((struct Dump *) dump_ptr == (void *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }

    /* Collega la memoria condivisa pids al puntatore*/
    pids_ptr = (struct Atomi *)shmat(shmid, NULL, 0);
    if (pids_ptr == (void *)-1) {
        perror("shmat attivatore puntatore pids");
        return 1;
    }

    printf("Attivatore waiting for zero... \n");

    sem_wait_for_zero(semid);

    /*sleep(STEP_ATTIVATORE);*/
    while(1) {
        i=0;
        int casuali[3];
        P(7, semid );
        for(int k = 0; k < 3; k++ ){
            casuali[k] = random_btw(0, pids_ptr->indice);
        }
        /*printf("Attivatore stampa indiceeeee %d\n", pids_ptr->indice );
        for(int f = 0; f <pids_ptr->indice; f++){
            printf("PID LETTI :%d con i %d \n", pids_ptr->pids[f], f);
        }*/
        while (i < 3) {
            myMessage.msg_type = pids_ptr->pids[casuali[i]];
            num_bytes = sprintf(myMessage.msg_text, "scissione");
            msgsnd(msgid, &myMessage, num_bytes, 0);
            P(1, semid);
            dump_ptr->attivazione = dump_ptr->attivazione + 1;
            V(1, semid);
            i++;
        }
        V(7, semid);
        sleep(STEP_ATTIVATORE);
    }


}



void gestore_segnale() {
    /*detach memoria condivisa per array pid*/
    if (shmdt(pids_ptr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    /* detach memoria condivisa dal processo*/
    if (shmdt(dump_ptr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    printf("Detach in attivatore avvenuto\n");
    exit(EXIT_SUCCESS);
}
