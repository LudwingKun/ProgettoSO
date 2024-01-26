#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/errno.h>
#include <string.h>
#include <sys/shm.h>
#include "lib/macro.h"
#include "lib/Util.h"
#include "lib/MacroUtil.h"
#include "lib/GeneralUtils.h"



/*Funziona però meglio se si trova una maniera migliore di strutturare il
 * codice perchè sono un sacco di parametri!!*/
struct Dump* dump_ptr;
int *pids_ptr;
int main(int argc, char *argv[]) {
    int MIN_N_ATOMICO;
    int msgid;
    int fd[2];
    struct message myMessage;
    int num_bytes;
    int numero_atomico, num_figlo_i, num_figlio_r, shmidInit;
    int *shmidInit_ptr;
    int semid;
    int pids_id;
    int dump_id;
    int pos;


    if (argc != 3) {
        fprintf(stderr, "Error, wrong number of argument for atomo\n");
        exit(EXIT_FAILURE);
    }

    shmidInit = atoi(argv[1]);
    numero_atomico = atoi(argv[2]);
    printf("numero atomico dell'atomo %d e' ==== %d\n", getpid(), numero_atomico);

    /*Collega la memoria condivisa Init al puntatore*/
    shmidInit_ptr = (int *) shmat(shmidInit, NULL, 0);
    if ((int *) shmidInit_ptr == (void *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }

    MIN_N_ATOMICO = shmidInit_ptr[1];
    printf("MIN_N_ATOMICO: %d", MIN_N_ATOMICO);
    msgid = shmidInit_ptr[11];
    printf("msgid: %d", msgid);
    semid = shmidInit_ptr[10];
    pids_id = shmidInit_ptr[8];
    dump_id = shmidInit_ptr[9];

    pids_ptr = (int *) shmat(pids_id, NULL, 0);
    if ((int *) pids_ptr == (void *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }
    dump_ptr = (struct Dump *) shmat(dump_id, NULL, 0);
    if ((struct Dump *) dump_ptr == (struct Dump *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }
    /* detach memoria condivisa per array pid*/
    if (shmdt(shmidInit_ptr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }


    msgid = msgget(MSGKEY, 0);
    if (msgid == -1) {
        fprintf(stderr, "%s: %d. Errore in msgget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("msgid sono in atomo: %d\n", msgid);

    printf("Atomo %d waiting for 0 before fire execve \n", getpid());
    sem_wait_for_zero(semid);

    while (1) {
        num_bytes = 0;
        num_bytes = msgrcv(msgid, &myMessage, 100, getpid(), 0); /*1 sarebbe il tipo del messaggio */
        if (num_bytes < 0) {
            if (errno == ENOMSG) {
                printf("CHILD %d: nessun messaggio di quel tipo", getpid());
                sleep(2);
                continue;
            } else {
                printf("Errore n %d: %s", errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        if (num_bytes > 0) {
            printf("messaggio ricevuto per pid %d: %s\n\n", getpid(), myMessage.msg_text);
            if (numero_atomico < MIN_N_ATOMICO) {
                printf("CHILD %d va in scorie e termina \n", getpid());
                P(5, semid);
                dump_ptr->scoria_prodotta++;
                V(5, semid);

                P(6, semid);
                pos = trova_pos(pids_ptr, &dump_ptr->contatore_atomi, getpid());
                printf("DIM VETTORE prima : %d \n", dump_ptr->contatore_atomi);
                printf("POS TROVATA .%d \n", pos);
                rimuovi_elemento(pids_ptr, &dump_ptr->contatore_atomi, pos);
                printf("DIM VETTORE: %d \n", dump_ptr->contatore_atomi);
                V(6, semid);
                exit(EXIT_SUCCESS);
            }
            if (pipe(fd) == -1) {
                perror("fail in pipe");
                TEST_ERROR
                exit(EXIT_FAILURE);
            }
            switch (fork()) {
                case -1:
                    TEST_ERROR
                    exit(EXIT_FAILURE);
                case 0:
                    printf("CHILD %d divisione da atomo : %d  \n ", getpid(), getppid());
                    lettura_pipe(fd, &num_figlio_r);
                    /**aggiornare DUMP**/
                    printf("CHILD %d con numero atomico : %d \n", getpid(), num_figlio_r);
                    break;
                default:
                    num_figlo_i = numero_atomico - numero_atomico / 2;
                    numero_atomico = numero_atomico - num_figlo_i;
                    scrittura_pipe(fd, &num_figlo_i);
                    break;
            }
            close(fd[1]);
        }

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
    printf("Detach in attivatore avvenuto");
    exit(EXIT_SUCCESS);
}



