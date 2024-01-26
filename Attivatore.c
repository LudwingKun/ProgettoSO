#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/errno.h>
#include <string.h>
#include "lib/GeneralUtils.h"
#include <sys/shm.h>
#include <signal.h>

int *puntatore_pids;
struct Dump *dump_ptr;




int main(int argc, char *argv[]) {
    int msgid, shmid, shmidReport,i=0;
    struct message myMessage;
    int shmidInit, STEP_ATTIVATORE;
    int *shmidInit_ptr;
    int num_bytes;
    int semid;
    struct sigaction sa;
    sa.sa_handler = gestore_segnale;
    sigaction(SIGTERM, &sa, NULL);


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
    printf("STEP_ATTIVATORE: %d\n", STEP_ATTIVATORE);
    semid = shmidInit_ptr[10];
    printf("semid: %d\n", semid);
    shmidReport = shmidInit_ptr[9];
    printf("shmidReport: %d\n", shmidReport);
    shmid = shmidInit_ptr[8];
    printf("shmid: %d\n", shmid);
    msgid = shmidInit_ptr[11];
    printf("msgid: %d\n", msgid);



    /* detach memoria condivisa per init */
    if (shmdt(shmidInit_ptr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }



    /*Collega la memoria condivisa Report al puntatore*/
    dump_ptr = (struct Dump *) shmat(shmidReport, NULL, 0);
    if ((struct Dump *) dump_ptr == (struct Dump *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }

    /* Collega la memoria condivisa pids al puntatore*/
    puntatore_pids = (int*)shmat(shmid,NULL,0);
    if (puntatore_pids == (void *)-1) {
        perror("shmat attivatore puntatore pids");
        return 1;
    }

    printf("Attivatore waiting for zero...");
    sem_wait_for_zero(semid);

    printf("!!!!!LETTURA pid da ATTIVATORE: %d\n",puntatore_pids[0]);
    printf("!!!!!LETTURA pid da ATTIVATORE: %d\n",puntatore_pids[1]);
    printf("!!!!!LETTURA pid da ATTIVATORE: %d\n",puntatore_pids[2]);
    

    printf("msgid sono in ATTIVATORE: %d\n",msgid);


    sleep(STEP_ATTIVATORE);
    while(1) {
        i=0;
        while (puntatore_pids[i] !=NULL) {
            myMessage.msg_type = puntatore_pids[i];
            num_bytes = sprintf(myMessage.msg_text, "scissione");
            msgsnd(msgid, &myMessage, num_bytes, 0);
            P(1, semid);
            dump_ptr->attivazione = dump_ptr->attivazione + 1;
            V(1, semid);
            i++;
        }
        sleep(STEP_ATTIVATORE);

    }


}





void gestore_segnale() {
    /*detach memoria condivisa per array pid*/
    if (shmdt(puntatore_pids) == -1) {
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
