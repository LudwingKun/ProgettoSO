#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/msg.h>
#include "lib/MacroUtil.h"
#include "lib/Util.h"
#include "lib/macro.h"
#include "lib/GeneralUtils.h"




int main(int argc, char *argv[]) {
    /**************************DICHIARAZIONI DI VARIABILI QUI !!*************************************************/
    int i;
    int pid_attivatore, pid_alimentazione;
    int N_ATOMI_INIT, ENERGY_DEMAND, MIN_N_ATOMICO, N_NUOVI_ATOMI, N_ATTIVATORE, N_ALIMENTAZIONE, N_ATOM_MAX, STEP_ALIMENTAZIONE, ENERGY_EXPLODE_THRESHOLD, SIM_DURATION, STEP_ATTIVATORE;
    /*int atomi_pids[20]; array di pid degli atomi che dovrà esssere istanziato con malloc perchè variano i numeri degli atomi creati!*/
    /*char *args1[4];
    char *args2[5];
    char *args3[6]; */
    int current_second;
    int semid, shmid, msgid, shmidReport;
    int *atomi_pids;
    struct Dump dump = {0, 0, 0, 0, 0};
    struct Dump *shared_dump;

    int numero_atomico; /*num spedito dal padre*/
    int num; /*num letto dall'atomo*/
    int file_pipe[2];

    char buffer[10] = "";
    char buffer2[10] = "";
    char buffer3[10] = "";
    char buffer4[10] = "";
    char buffer5[10] = "";
    char buffer6[10] = "";
    char buffer7[10] = "";/*problemi con le dimennsioni del buffer per il dump*/
    /*char *filePath_atomo = "atomo";
    char *filePath_attivatore = "attivatore";
    char *filePath_alimentazione = "alimentazione";*/


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

    printf("SIM_DURATION %d\n\n", SIM_DURATION);

    /*ora i parametri sono nelle variabili ma come faccio a passargli agli altri processi o a renderle visibili agli altri processi?
     * */

    /*idea: posso trasformare i parametri del file in char e poi riconvertirli in interi nei file dei processi!*/



    /*************************************** CREAZIONE STRUTTURE *****************************************************/
    atomi_pids = malloc(N_ATOMI_INIT + N_NUOVI_ATOMI * sizeof(*atomi_pids));

    /* Creazione MEMORIA CONDIVISA per array pid */
    shmid = shmget(SHMKEY, sizeof(int) * (N_ATOMI_INIT + N_NUOVI_ATOMI), IPC_CREAT | 0666);
    if (shmid == -1) {
        fprintf(stderr, "%s: %d. Errore in shmget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("....shmid sono in MASTER: %d\n", shmid);

    /* Collega la memoria condivisa al puntatore*/
    atomi_pids = (int*)shmat(shmid,NULL,0);
    if (atomi_pids == (void *)-1) {
        perror("shmat");
        return 1;
    }

    /* Creazione MEMORIA CONDIVISA per le statistiche ogni secondo, metto struttura DUMP! */
    shmidReport = shmget(SHMKEYREPORT, sizeof(struct Dump), IPC_CREAT | 0666);
    if (shmidReport == -1) {
        fprintf(stderr, "%s: %d. Errore in shmget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    shared_dump = (struct Dump *) shmat(shmidReport, NULL, 0);
    if ((struct Dump *) shared_dump == (struct Dump *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }

    /* creazione SEMAFORO per scrittura e lettura */
    semid = semget(SEMWRITEKEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        fprintf(stderr, "%s: %d. Errore in semget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }


    /* Creazione message queue */
    msgid = msgget(MSGKEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        fprintf(stderr, "%s: %d. Errore in msgget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("....msgid sono in MASTER: %d\n", msgid);




    /*************************************** FINE CREAZIONE STRUTTURE *****************************************************/
    /* Proviamo a trasformare STEP_ATTIVATORE E SIM_DURATION! da int a char per passarli! */


    /* Utilizza sprintf per convertire l'intero in una stringa */
    sprintf(buffer, "%d", STEP_ATTIVATORE);
    sprintf(buffer2, "%d", SIM_DURATION);
    sprintf(buffer3, "%d", STEP_ALIMENTAZIONE);
    sprintf(buffer4, "%d", N_NUOVI_ATOMI);
    sprintf(buffer5, "%d", N_ATOM_MAX);
    sprintf(buffer6, "%d", shmid);
    sprintf(buffer7, "%d", shared_dump);


    /*printf("Il numero intero come stringa: %s\n", buffer);*/
    char *args1[] = {"atomo", buffer, buffer2, buffer,NULL};
    char *args2[] = {"attivatore", buffer, buffer2, buffer6,/*buffer7*/NULL};
    char *args3[] = {"alimentazione", buffer2, buffer3, buffer4, buffer5, NULL};

    /*
    args1[0] = "atomo";
    args1[1] = buffer;
    args1[2] = buffer2;
    args1[3] = NULL;

    args2[0] = "attivatore";
    args2[1] = buffer;
    args2[2] = buffer2;
    args2[3] = buffer6;
    args2[4] = NULL;

    args3[0] = "alimentazione";
    args3[1] = buffer2;
    args3[2] = buffer3;
    args3[3] = buffer4;
    args3[4] = buffer5;
    args3[5] = NULL; */

    srand(time(NULL));

    /*inizializzo il semaforo*/
    sem_init(semid, N_ATTIVATORE + N_ATOMI_INIT + N_ALIMENTAZIONE + 1);
    for (i = 0; i < N_ATOMI_INIT; i++) {
        if(pipe(file_pipe) == -1){
            perror("fail in pipe");
            exit(EXIT_FAILURE);
        }


/**TODO: non é una doppia dichiarazione ? **/

        switch (fork()) {
            case -1:
                perror("fork fail for atom");
                exit(EXIT_FAILURE);
            case 0:
                sem_down(semid); /*decremento del semaforo*/
                printf("Atomo creato!\npid: %d \npadre: %d \n", getpid(), getppid());

                /*ricezione numero atomico*/
                close(file_pipe[1]); /*chiudi scrittura*/
                sleep(1);
                read(file_pipe[0], &num, sizeof(int));
                printf("numero letto: %d per atomo con pid: %d\n", num, getpid());
                close(file_pipe[0]);
                TEST_ERROR
                sprintf(args1[3], "%d", num); /*conversione a stringa*/

                printf("!!!!LETTURA Atomo_pid da MASTER: %d\n", getpid());
                atomi_pids[i]=getpid();

                printf("Atomo %d waiting for 0 before fire execve \n", getpid());
                sem_wait_for_zero(semid);
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
                write(file_pipe[1], &numero_atomico, sizeof(int) );
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

            printf("Attivatore waiting for zero...");
            sem_wait_for_zero(semid);
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
            printf("Alimentazione waiting for zero");
            sem_wait_for_zero(semid);

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
        printf("Attivazioni dell'Attivatore: %d\n", dump.attivazione);
        printf("Scissioni al secondo: %d\n", dump.scissione);
        printf("Energia Prodotta al secondo: %d\n", dump.energia_prodotta);
        printf("Energia Consumata al secondo: %d\n", dump.energia_consumata);
        printf("Scorie prodotte al secondo: %d\n\n\n", dump.scoria_prodotta);

        /*************************** STATISTICHE (totali) **********************************************************/
        /* In realtà vedi se dividere e se non c'è bisogno */
        sleep(1);
    }

    sleep(5); /*Questa sleep serve sennò il master finisce prima degli altri processi*/



    /* Rimozione dei semafori e della memoria condivisa */

    /*detach memoria condivisa per array pid*/
    if (shmdt(atomi_pids) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    /* detach memoria condivisa dal processo */
    if (shmdt(shared_dump) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    /* rimozione memoria condivisa */
    if (shmctl(shmidReport, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }


    semctl(semid, 0, IPC_RMID);
    shmctl(shmid, IPC_RMID, NULL);
    msgctl(msgid, IPC_RMID, NULL);

    printf("END!\n");

    return 0;


}


