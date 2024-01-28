#include "lib/MacroUtil.h"
#include "lib/GeneralUtils.h"

struct Dump *dump_ptr;
atomi *pids_ptr;


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
    struct sigaction sa;
    sa.sa_handler = gestore_segnale;
    sigaction(SIGTERM, &sa, NULL);


    char buffer[10];
    char buffer1[10];
    char *args1[] = {"atomo",buffer, buffer1, NULL};
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
    sprintf(buffer, "%d", shmidInit);
    MIN_N_ATOMICO = shmidInit_ptr[1];
    msgid = shmidInit_ptr[11];
    semid = shmidInit_ptr[10];
    pids_id = shmidInit_ptr[8];
    dump_id = shmidInit_ptr[9];

    pids_ptr = (atomi *) shmat(pids_id, NULL, 0);
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
            /*printf("messaggio ricevuto per pid %d: %s\n\n", getpid(), myMessage.msg_text);*/
            if (numero_atomico <= MIN_N_ATOMICO) {
                printf("CHILD %d va in scorie e termina \n", getpid());

                P(5, semid);
                dump_ptr->scoria_prodotta++;
                V(5, semid);



                P(7, semid);
                pos = trova_pos(pids_ptr->pids, &pids_ptr->indice, getpid());
                printf("DIM VETTORE prima : %d \n", pids_ptr->indice);
                printf("POS TROVATA .%d \n", pos);
                rimuovi_elemento(pids_ptr->pids, &pids_ptr->indice, pos);
                printf("DIM VETTORE: %d \n", pids_ptr->indice);
                V(7, semid);

                gestore_segnale();
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
                    P(7,semid);
                    aggiungi_elemento(pids_ptr->pids, &pids_ptr->dimensione, &pids_ptr->indice , getpid());
                    V(7,semid);

                    P(2, semid);
                    dump_ptr->scissione++;
                    V(2, semid);
                    printf("CHILD %d con numero atomico : %d \n", getpid(), num_figlio_r);
                    sprintf(args1[2], "%d", num_figlio_r);
                    if (execve("bin/atomo", args1, NULL) == -1) {
                        perror("execve");
                        exit(EXIT_FAILURE);
                    }
                    exit (EXIT_FAILURE);
                    break;
                default:
                    num_figlo_i = numero_atomico - numero_atomico / 2;
                    numero_atomico = numero_atomico - num_figlo_i;
                    printf("ATOMO %d ha nuovo num atomico %d \n", getpid(), numero_atomico);
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
    printf("Detach in atomo avvenuto\n");
    exit(EXIT_SUCCESS);
}



