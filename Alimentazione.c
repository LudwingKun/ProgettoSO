#include "lib/GeneralUtils.h"


struct Dump *dump_ptr;
atomi *pids_ptr;
int main(int argc, char *argv[]) {
    int i;
    int atomi_alimentatore[10];
    int STEP_ALIMENTAZIONE, N_NUOVI_ATOMI, N_ATOM_MAX;
    char buffer[10];
    char buffer1[10];
    int numero_atomico; /*num spedito dal padre*/
    int num; /*num letto dall'atomo*/
    int file_pipe[2];
    int shmidInit, semid, dump_id, pids_id;
    int *shmidInit_ptr;
    struct sigaction sa;
    sa.sa_handler = gestore_segnale;
    sigaction(SIGTERM, &sa, NULL);
    struct timespec structtimespec;


    if (argc != 2) {
        fprintf (stderr, "Error, wrong number of argument for Alimentazione\n");
        exit (EXIT_FAILURE);
    }

    shmidInit = atoi(argv[1]);
    sprintf(buffer, "%d", shmidInit);

    /*Collega la memoria condivisa Init al puntatore*/
    shmidInit_ptr = (int *) shmat (shmidInit, NULL, 0);
    if ((int *) shmidInit_ptr == (void *) (-1)) {
        perror ("shmat fail");
        exit (EXIT_FAILURE);
    }

    STEP_ALIMENTAZIONE = shmidInit_ptr[5];
    N_NUOVI_ATOMI = shmidInit_ptr[6];
    N_ATOM_MAX = shmidInit_ptr[7];
    semid = shmidInit_ptr[10];
    dump_id = shmidInit_ptr[9];
    pids_id = shmidInit_ptr[8];

    /* detach memoria condivisa per init */
    if (shmdt (shmidInit_ptr) == -1) {
        perror ("shmdt");
        exit (EXIT_FAILURE);
    }
/**Collegamento allla memoria condivisa per i pids**/
    pids_ptr = (struct Atomi *)shmat(pids_id, NULL, 0);
    if (pids_ptr == (void *)-1) {
        perror("shmat Alimentazione puntatore pids");
        return 1;
    }

    /*Collega la memoria condivisa dump al puntatore*/
    dump_ptr = (struct Dump *) shmat(dump_id, NULL, 0);
    if ((struct Dump *) dump_ptr == (struct Dump *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }

    char *args1[] = {"atomo",buffer, buffer1, NULL};

    /*printf ("Sono dentro Alimentazione\n");
    printf ("pid alimentazione! %d\n", getpid ());*/

    printf ("Alimentazione waiting for zero\n");
    sem_wait_for_zero (semid);
    structtimespec.tv_nsec = STEP_ALIMENTAZIONE;
    structtimespec.tv_sec = 0;
    nanosleep(&structtimespec, NULL);
    while (1) {
        for (i = 0; i < N_NUOVI_ATOMI; i++) {
            if (pipe (file_pipe) == -1) {
                perror ("fail in pipe");
                exit (EXIT_FAILURE);
            }
            atomi_alimentatore[i] = fork();
            switch (atomi_alimentatore[i]) {
                case -1:
                    perror ("fork");
                    exit (EXIT_FAILURE);
                case 0:
                    P(7, semid);
                    aggiungi_elemento(pids_ptr->pids, &pids_ptr->dimensione, &pids_ptr->indice, getpid());
                    V(7, semid);

                    /*printf ("Atomo_Alimentatore_pid: %d\n", getpid());*/
                    P(6, semid);
                    dump_ptr->contatore_atomi++;
                    V(6, semid);

                    close (file_pipe[1]); /*chiudi scrittura*/
                    sleep (1);
                    read (file_pipe[0], &num, sizeof (int));
                    /*printf ("numero letto: %d per atomo con pid: %d\n", num, getpid());*/
                    close (file_pipe[0]);
                    /*TEST_ERROR*/
                    sprintf(args1[2], "%d", num);
                    if (execve("bin/atomo", args1, NULL) == -1) {
                        perror("execve");
                        exit(EXIT_FAILURE);
                    }
                    exit (EXIT_FAILURE);
                default:
                    /* PIPE -> atomo si mette in ascolto e attende il numero atomico che gli invia il padre */
                    close (file_pipe[0]);
                    numero_atomico = random_btw(1, N_ATOM_MAX);
                    /*printf ("PADRE ALIMENTAZIONE PRODUCE PER ALIMENTATORE: %d \n", numero_atomico);*/
                    write (file_pipe[1], &numero_atomico, sizeof (int));
                    break;
            }
        }
        close (file_pipe[1]);
        nanosleep(&structtimespec, NULL);
    }
}



void gestore_segnale() {

    /* detach memoria condivisa dal processo*/
    if (shmdt(dump_ptr) == -1) {
        perror("shmdt fail in Alimentazione");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
