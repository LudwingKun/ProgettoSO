#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/msg.h>
#include <signal.h>
#include "lib/MacroUtil.h"
#include "lib/GeneralUtils.h"


int main() {

    /**************************DICHIARAZIONI DI VARIABILI QUI !!*************************************************/
    int i;
    int N_ATOMI_INIT, ENERGY_DEMAND, MIN_N_ATOMICO, N_NUOVI_ATOMI, N_ATTIVATORE, N_ALIMENTAZIONE, N_ATOM_MAX, STEP_ALIMENTAZIONE, ENERGY_EXPLODE_THRESHOLD, SIM_DURATION, STEP_ATTIVATORE;
    int pid_attivatore, pid_alimentazione;
    int current_second;
    int shmidInit, semid, shmid, msgid, shmidReport;
    int *shmidInit_ptr;
    int *atomi_pids;
    struct Dump *shared_dump;


    int numero_atomico; /*num spedito dal padre*/
    int num; /*num letto dall'atomo*/
    int file_pipe[2];

    char buffer[10];
    char buffer2[10] = "dummy";

    char *args1[] = {"atomo", buffer, buffer2, NULL};
    char *args2[] = {"attivatore", buffer, NULL};
    char *args3[] = {"alimentazione", buffer, NULL};
    /*   char buffer3[10];
       char buffer4[10];
       char buffer5[10];
       char buffer6[10];
       char buffer7[10];
       char buffer8[10];*//*problemi con le dimennsioni del buffer per il dump*/

    /*************************************** LETTURA PARAMETRI *****************************************************/
    FILE *file = fopen("config_1.txt", "r");

    if (file == NULL) {
        perror("Errore nell'apertura del file");
        exit(EXIT_FAILURE);
    }
    if (fscanf(file,
               "N_ATOMI_INIT=%d\nN_ATTIVATORE=%d\nN_ALIMENTAZIONE=%d\nENERGY_DEMAND=%d\nN_ATOM_MAX=%d\nMIN_N_ATOMICO=%d\nSTEP_ATTIVATORE=%d\nSTEP_ALIMENTAZIONE=%d\nN_NUOVI_ATOMI=%d\nSIM_DURATION=%d\nENERGY_EXPLODE_THRESHOLD=%d\n",
               &N_ATOMI_INIT, &N_ATTIVATORE, &N_ALIMENTAZIONE, &ENERGY_DEMAND, &N_ATOM_MAX, &MIN_N_ATOMICO,
               &STEP_ATTIVATORE, &STEP_ALIMENTAZIONE, &N_NUOVI_ATOMI, &SIM_DURATION, &ENERGY_EXPLODE_THRESHOLD) != 11) {
        fprintf(stderr, "Errore nella lettura dei parametri di configurazione\n");
        exit(EXIT_FAILURE);
    }

    fclose(file);
    /*************************************** FINE LETTURA PARAMETRI *****************************************************/



    /*************************************** CREAZIONE STRUTTURE *****************************************************/

    atomi_pids = malloc(N_ATOMI_INIT + N_NUOVI_ATOMI * sizeof(int));

    /*Creazione memoria condivisa per passaggio parametri + ids*/
    shmidInit = shmget(IPC_PRIVATE, sizeof(int) * (15) + 1, IPC_CREAT | 0666);
    if (shmidInit == -1) {
        fprintf(stderr, "%s: %d. Errore in shmget for shmidInit #%03d: %s\n", __FILE__, __LINE__, errno,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    /*Collegamento memoria condivisa*/
    shmidInit_ptr = (int *) shmat(shmidInit, NULL, 0);
    if (shmidInit_ptr == (void *) -1) {
        perror("shmat");
        return 1;
    }

    /* Creazione MEMORIA CONDIVISA per array pid */
    shmid = shmget(IPC_PRIVATE, sizeof(int) * (N_ATOMI_INIT + N_NUOVI_ATOMI), IPC_CREAT | 0666);
    if (shmid == -1) {
        fprintf(stderr, "%s: %d. Errore in shmget for shmid #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("....shmid sono in MASTER: %d\n", shmid);

    /* Collega la memoria condivisa al puntatore*/
    atomi_pids = (int *) shmat(shmid, NULL, 0);
    if (atomi_pids == (void *) -1) {
        perror("shmat");
        return 1;
    }

    /* Creazione MEMORIA CONDIVISA per le statistiche ogni secondo, metto struttura DUMP! */
    shmidReport = shmget(IPC_PRIVATE, sizeof(struct Dump), IPC_CREAT | 0666);
    if (shmidReport == -1) {
        fprintf(stderr, "%s: %d. Errore in shmget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    shared_dump = (struct Dump *) shmat(shmidReport, NULL, 0);
    if ((struct Dump *) shared_dump == (struct Dump *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }

    /* creazione 7 semafori
     * 0)sincronizzazione processi iniziali
     * 1)attivazione
     * 2)scissione
     * 3)energia prodotta
     * 4)energia consumata
     * 5)scorie prodotta
     * 6)contatore atomi*/
    semid = semget(IPC_PRIVATE, 7, IPC_CREAT | 0666);
    if (semid == -1) {
        fprintf(stderr, "%s: %d. Errore in semget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("....semid in MASTER: %d\n", semid);


    /* Creazione message queue */
    msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    if (msgid == -1) {
        fprintf(stderr, "%s: %d. Errore in msgget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("....msgid sono in MASTER: %d\n", msgid);

    /*************************************** FINE CREAZIONE STRUTTURE *****************************************************/


    /* Fill memoria condivisa */
    shmidInit_ptr[0] = ENERGY_DEMAND;
    shmidInit_ptr[1] = MIN_N_ATOMICO;
    shmidInit_ptr[2] = ENERGY_EXPLODE_THRESHOLD;
    shmidInit_ptr[3] = STEP_ATTIVATORE;
    shmidInit_ptr[4] = SIM_DURATION;
    shmidInit_ptr[5] = STEP_ALIMENTAZIONE;
    shmidInit_ptr[6] = N_NUOVI_ATOMI;
    shmidInit_ptr[7] = N_ATOM_MAX;
    shmidInit_ptr[8] = shmid;
    shmidInit_ptr[9] = shmidReport;
    shmidInit_ptr[10] = semid;
    shmidInit_ptr[11] = msgid;
/*
   sprintf(buffer, "%d", shmidInit);*/

    if (sprintf((char *) args1[1], "%d", shmidInit) < 0) {
        printf("ERRORE");
    }


    srand(time(NULL));

    /*Inizializzo semaforo per dump*/
    /*  sem_init(semid, 1, 1);*/
    sem_init(semid, N_ATTIVATORE + N_ATOMI_INIT + N_ALIMENTAZIONE + 1, 0);
    for (int j = 1; j < 7; j++)
        sem_init(semid, 1, j);
    printf("YOIOOOOOOO\n");
    /*inizializzo il semaforo*/

    P(6, semid);
    shared_dump->contatore_atomi = 0;
    V(6, semid);
    for (i = 0; i < N_ATOMI_INIT; i++) {
        if (pipe(file_pipe) == -1) {
            perror("fail in pipe");
            exit(EXIT_FAILURE);
        }


        switch (fork()) {
            case -1:
                perror("fork fail for atom");
                exit(EXIT_FAILURE);
            case 0:
                sem_down(semid); /*decremento del semaforo*/
                printf("Atomo creato!\npid: %d \npadre: %d \n", getpid(), getppid());
                P(6, semid);
                shared_dump->contatore_atomi++;
                V(6, semid);

                /*ricezione numero atomico*/
                close(file_pipe[1]); /*chiudi scrittura*/
                sleep(1);
                read(file_pipe[0], &num, sizeof(int));
                printf("numero letto: %d per atomo con pid: %d\n", num, getpid());
                close(file_pipe[0]);
                TEST_ERROR
                sprintf((char *) args1[2], "%d", num); /*conversione a stringa*/

                printf("!!!!LETTURA Atomo_pid da MASTER: %d\n", getpid());
                atomi_pids[i] = getpid();

                /*printf("Atomo %d waiting for 0 before fire execve \n", getpid());
                sem_wait_for_zero(semid);*/
                if (execve("bin/atomo", args1, NULL) == -1) {
                    perror("execve");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_FAILURE);
            default:
                /* PIPE -> atomo si mette in ascolto e attende il numero atomico che gli invia il padre */
                close(file_pipe[0]);
                numero_atomico = random_btw(1, N_ATOM_MAX);
                printf("PADRE PRODUCE : %d \n", numero_atomico);
                write(file_pipe[1], &numero_atomico, sizeof(int));
                break;
        }

    }

    close(file_pipe[1]);


    pid_attivatore = fork();
    switch (pid_attivatore) {
        case -1:
            perror("error fork attivatore");
            exit(EXIT_FAILURE);
        case 0:
            sem_down(semid);
            printf("Attivatore con pid %d \n", getpid());
            /*printf("Attivatore waiting for zero...");
            sem_wait_for_zero(semid);*/
            if (execve("bin/attivatore", args2, NULL) == -1) {
                perror("execve");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_FAILURE);
        default:
            break;
    }

    /*sleep(10);*/

    pid_alimentazione = fork();
    switch (pid_alimentazione) {
        case -1:
            perror("error fork alimentazione");
            exit(EXIT_FAILURE);
        case 0:
            sem_down(semid);
            printf("Alimentazione con pid %d", getpid());
            /*printf("Alimentazione waiting for zero");
            sem_wait_for_zero(semid);*/

            if (execve("bin/alimentazione", args3, NULL) == -1) {
                perror("execve");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_FAILURE);
        default:
            break;
    }

    sleep(5);
    sem_down(semid);
    sem_wait_for_zero(semid);

    printf("START!\n");

    for (current_second = 0; current_second <= SIM_DURATION; current_second++) {
        /*kill processi atomo*/
        /*for(i = 0; i < N_ATOMI_INIT; i++) {
            printf("atomi_pids[%d] = %d\n", i, atomi_pids[i]);
            kill(atomi_pids[i], SIGKILL);
        }*/
        /*************************** STATISTICHE (al sec) **********************************************************/
        printf("Secondo %d\n", current_second);
        printf("Attivazioni dell'Attivatore: %d\n", shared_dump->attivazione);
        printf("Scissioni al secondo: %d\n", shared_dump->scissione);
        printf("Energia Prodotta al secondo: %d\n", shared_dump->energia_prodotta);
        printf("Energia Consumata al secondo: %d\n", shared_dump->energia_consumata);
        printf("Scorie prodotte al secondo: %d\n", shared_dump->scoria_prodotta);
        printf("Numero atomi totali: %d \n\n\n", shared_dump->contatore_atomi);

        /*************************** STATISTICHE (totali) **********************************************************/
        /* In realtà vedi se dividere e se non c'è bisogno */
        sleep(1);
    }

    //sigemtyset

    if (kill(pid_attivatore, SIGTERM) == 0) {
        printf("Segnale di terminazione inviato al processo con PID %d\n", pid_attivatore);
    } else {
        perror("Errore durante l'invio del segnale");
        return 1;
    }

    if (kill(pid_alimentazione, SIGTERM) == 0) {
        printf("Segnale di terminazione inviato al processo con PID %d\n", pid_alimentazione);
    } else {
        perror("Errore durante l'invio del segnale");
        return 1;
    }
    sleep(3);
    /**************/
    /*for (int k = 0; k < shared_dump->contatore_atomi; k++) {
        if (kill(atomi_pids[k], SIGTERM) == 0) {
        } else {
            perror("Errore durante l'invio del segnale");
            return 1;
        }
    }*/


    sleep(5); /*Questa sleep serve sennò il master finisce prima degli altri processi*/



    /*------------------Rimozione dei semafori e della memoria condivisa---------------------------- */

    /*detach memoria condivisa per array pid*/
    if (shmdt(atomi_pids) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    /* detach memoria condivisa dal processo per dump */
    if (shmdt(shared_dump) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    /* detach memoria condivisa dal processo per Init */
    if (shmdt(shmidInit_ptr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    /* rimozione memoria condivisa per dump */
    if (shmctl(shmidReport, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }


    semctl(semid, 0, IPC_RMID);
    semctl(semid, 1, IPC_RMID);
    shmctl(shmid, IPC_RMID, NULL);
    shmctl(shmidInit, IPC_RMID, NULL);
    msgctl(msgid, IPC_RMID, NULL);

    printf("END!\n");

    return 0;


}
