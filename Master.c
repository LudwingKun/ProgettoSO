#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <time.h>

#include "macro.h"
#include "util.h"


void seedRandom() {
    struct timespec ts;
    unsigned long seed;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("clock_gettime");
        exit(1);
    }
    seed = ((unsigned long)ts.tv_sec * 1000000000) + ts.tv_nsec;
    srand(ts.tv_nsec+seed);
}

/**Funzione per prendere un numero random per il Numero Atomico degli atomi (quando padre li crea) */

int getRandomNumber(int lowerLimit, int upperLimit) { /*questa va chiamata con lowerLimit = 1 e upperLimit = N_ATOM_MAX */
    int randomNumber;
    seedRandom();
    randomNumber= rand() % (upperLimit - lowerLimit +1) + lowerLimit;

    return randomNumber;
}


int main() {
	int i;
	int pid, pid_attivatore, pid_alimentazione;
	int atomi_pids[20];
	char *argv[] = {NULL}, *envp[] = {NULL}; 
	int status;

	/*************************************** CREAZIONE STRUTTURE *****************************************************/
	int semid, shmid, msgid, semReadId;

	shmid = shmget(SHMKEY, sizeof(int) * (N_ATOMI_INIT + N_NUOVI_ATOMI), IPC_CREAT | 0666);
	if (shmid == -1) {
        fprintf(stderr, "%s: %d. Errore in shmget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* creazione SEMAFORO per scrittura */
    semid = semget(SEMWRITEKEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        fprintf(stderr, "%s: %d. Errore in semget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* imposto valore del semaforo a 1 scrittura */
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        fprintf(stderr, "%s: %d. Errore in semctl #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Creazione del semaforo per la lettura */
    semReadId = semget(SEMREADKEY, 1, IPC_CREAT | 0666);
    if (semReadId == -1) {
        fprintf(stderr, "%s: %d. Errore in semget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* imposto valore del semaforo a 0 lettura */
    if (semctl(semReadId, 0, SETVAL, 0) == -1) {
        fprintf(stderr, "%s: %d. Errore in semctl #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }


    /* Creazione message queue */
    msgid = msgget(MSGKEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        fprintf(stderr, "%s: %d. Errore in msgget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }



	/*************************************** FINE CREAZIONE STRUTTURE *****************************************************/



	for (i=0;i<N_ATOMI_INIT;i++) {
		atomi_pids[i] = fork();
        switch (atomi_pids[i]) {
            case -1:
                perror("fork");
                exit(EXIT_FAILURE);
            case 0:
                printf("Atomo creato!\n");
                printf("Atomo_pid: %d\n", getpid());
                printf("padre atomo: %d\n", getppid());
                execve("/Users/ari3/Desktop/ProgettoSO/atomo", argv, envp);
                if(execve("/Users/ari3/Desktop/ProgettoSO/atomo", argv, envp) == -1) {
                	perror("execve");
                	exit(EXIT_FAILURE);
                }
                exit(EXIT_FAILURE);
            default:
            	atomo.numero_atomico = getRandomNumber(1, N_ATOM_MAX);
            	atomo.pid = atomi_pids[i];
            	printf("Atomo con pid %d ha questo numero atomico: %d\n\n", atomo.pid, atomo.numero_atomico);
                /*Codice del processo padre non so se è vero. controlla!*/
                /*atomi_pids[i] = waitpid(pid, &status, 0);

                if(atomi_pids[i] == -1) {
                	perror("waitpid");
                	exit(EXIT_FAILURE);
                }

                if(WIFEXITED(status)) {
                	printf("Il processo figlio con PID %d e' terminato con stato %d\n", pid, WEXITSTATUS(status));

                } else {
                	printf("Il processo figlio con PID %d non e' terminato correttamente", pid);
                }*/
                break;
        }
    }

    for (i=0;i<N_ATOMI_INIT;i++){
        printf("Atomo %d: %d\n", i, atomi_pids[i]);
    }

    sleep(10);

    pid_attivatore = fork();
    switch (pid_attivatore) {
        case -1:
            perror("error fork attivatore");
            exit(EXIT_FAILURE);
        case 0:
            execve("/Users/ari3/Desktop/ProgettoSO/attivatore", argv, envp);
            if(execve("/Users/ari3/Desktop/ProgettoSO/attivatore", argv, envp) == -1) {
                perror("execve");
               	exit(EXIT_FAILURE);
            }
            exit(EXIT_FAILURE);
        default:
            break;
    }

    sleep(10);

    pid_alimentazione = fork();
    switch (pid_alimentazione) {
        case -1:
            perror("error fork alimentazione");
            exit(EXIT_FAILURE);
        case 0:
            execve("/Users/ari3/Desktop/ProgettoSO/Alimentazione", argv, envp);
            if(execve("/Users/ari3/Desktop/ProgettoSO/Alimentazione", argv, envp) == -1) {
                perror("execve");
               	exit(EXIT_FAILURE);
            }
            exit(EXIT_FAILURE);
        default:
            break;
    }

    sleep(10);
    printf("START!\n");
    sleep(10);

    /* Rimozione dei semafori e della memoria condivisa */
    semctl(semid, 0, IPC_RMID);
    semctl(semReadId, 0, IPC_RMID);
    shmctl(shmid, IPC_RMID, NULL);
    msgctl(msgid, IPC_RMID, NULL);

    printf("END!\n");

    return 0;
            
           

} 