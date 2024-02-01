#include "lib/GeneralUtils.h"


struct Atomi *pids_ptr;
struct Dump *dump_ptr;
message myMessage;
int msgid;
int main(int argc, char *argv[]) {

    int shmid, shmidReport,i=0;
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
        perror("shmat shmidInit_ptr fail in attivatore");
        exit(EXIT_FAILURE);
    }

    STEP_ATTIVATORE = shmidInit_ptr[3];
    semid = shmidInit_ptr[10];
    shmidReport = shmidInit_ptr[9];
    shmid = shmidInit_ptr[8];
    msgid = shmidInit_ptr[11];



    /* detach memoria condivisa per init */
    if (shmdt(shmidInit_ptr) == -1) {
        perror("shmdt shmidInit_ptr fail in attivatore");
        exit(EXIT_FAILURE);
    }



    /*Collega la memoria condivisa Report al puntatore*/
    dump_ptr = (struct Dump *) shmat(shmidReport, NULL, 0);
    if ((struct Dump *) dump_ptr == (void *) (-1)) {
        perror("shmat puntatore dump fail in attivatore");
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

/*    sleep(STEP_ATTIVATORE);*/
    while(1) {
        i=0;
        /*int casuali[10];*/
    /*    P(7, semid );
        for(int k = 0; k < 10; k++ ){
            casuali[k] = random_btw(0, pids_ptr->indice);
        }*/
        while (i < 10) {
            myMessage.msg_type = 1; /**messaggio dell'attivatore*/
            myMessage.msg_text = 1;/**comando dell'attivatore**/
            msgsnd(msgid, &myMessage, sizeof(int), 0);
            P(1, semid);
            dump_ptr->attivazione = dump_ptr->attivazione + 1;
            V(1, semid);
            i++;
        }
/*        V(7, semid);*/
        sleep(STEP_ATTIVATORE);
    }


}



void gestore_segnale() {
    /*detach memoria condivisa per array pid*/
    myMessage.msg_type = 1;
    myMessage.msg_text = 0;
    msgsnd(msgid, &myMessage, sizeof(int), 0);
    if (shmdt(pids_ptr) == -1) {
        perror("shmdt puntatore pids fail in attivatore");
        exit(EXIT_FAILURE);
    }

    /* detach memoria condivisa dal processo*/
    if (shmdt(dump_ptr) == -1) {
        perror("shmdt puntatore dump fail in attivatore");
        exit(EXIT_FAILURE);
    }
    printf("Detach in attivatore avvenuto\n");
    exit(EXIT_SUCCESS);
}
