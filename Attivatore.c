#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/errno.h>
#include <string.h>
#include "lib/macro.h"
#include "lib/Util.h"
#include "lib/GeneralUtils.h"
#include <sys/shm.h>


int main(int argc, char *argv[]) {
    int msgid, shmid, shmidReport,i=0;
    struct message myMessage;
    int shmidInit, STEP_ATTIVATORE;
    int *shmidInit_ptr;
    int *puntatore_pids;
    struct Dump *shared_dump;
    int num_bytes;
    int semid;

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
    shared_dump = (struct Dump *) shmat(shmidReport, NULL, 0);
    if ((struct Dump *) shared_dump == (struct Dump *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }

    /* Collega la memoria condivisa pids al puntatore*/
    puntatore_pids = (int*)shmat(shmid,NULL,0);
    if (puntatore_pids == (void *)-1) {
        perror("shmat attivatore puntatore pids");
        return 1;
    }

    printf("!!!!!LETTURA pid da ATTIVATORE: %d\n",puntatore_pids[0]);
    printf("!!!!!LETTURA pid da ATTIVATORE: %d\n",puntatore_pids[1]);
    printf("!!!!!LETTURA pid da ATTIVATORE: %d\n",puntatore_pids[2]);
    

    msgid = msgget(MSGKEY, 0);
    if (msgid == -1) {
        fprintf(stderr, "%s: %d. Errore in msgget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("msgid sono in ATTIVATORE: %d\n",msgid);

    /*detach memoria condivisa per array pid*/
    if (shmdt(puntatore_pids) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    /* detach memoria condivisa dal processo */
    if (shmdt(shared_dump) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }


    while(1) {
        while (puntatore_pids[i] != NULL) {
            myMessage.msg_type = puntatore_pids[1];
            num_bytes = sprintf(myMessage.msg_text, "scissione");
            msgsnd(msgid, &myMessage, num_bytes, 0);
            P(1, semid);
            shared_dump->attivazione = shared_dump->attivazione + 1;
            printf("shared_dump->attivazione:  %d \n\n", shared_dump->attivazione);
            V(1, semid);
            i++;
        }
        sleep(STEP_ATTIVATORE);
    }




}
