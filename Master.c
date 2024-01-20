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
#include "Util.h"
#include "SemaforiUtil.h"

void seedRandom() {
    struct timespec ts;
    unsigned long seed;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("clock_gettime");
        exit(1);
    }
    seed = ts.tv_nsec;
    srand(seed);
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
	int atomi_pids[N_ATOMI_INIT];
	char *argv[] = {NULL}, *envp[] = {NULL}; 
	int status;
    int current_second;
	/*************************************** CREAZIONE STRUTTURE *****************************************************/
	int semid, shmid, msgid, semReadId;

	shmid = shmget(SHMKEY, sizeof(int) * (N_ATOMI_INIT + N_NUOVI_ATOMI), IPC_CREAT | 0666);
	if (shmid == -1) {
        fprintf(stderr, "%s: %d. Errore in shmget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* creazione SEMAFORO per scrittura e lettura */
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
    /*todo: questo serve ? secondo me no:)*/
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

    /*inizializzo il semaforo*/
    sem_init(semid);
	for (i=0;i<N_ATOMI_INIT;i++) {
		atomi_pids[i] = fork();
        switch (atomi_pids[i]) {
            case -1:
                perror("fork");
                exit(EXIT_FAILURE);
            case 0:
                sem_down(semid); /*decremento del semaforo*/

                printf("Atomo creato!\n");
                printf("Atomo_pid: %d\n", getpid());
                printf("padre atomo: %d\n", getppid());

                printf("Atomo %d waiting for 0 before fire execve \n", getpid());
                sem_wait_for_zero(semid);
                execve("atomo", argv, envp);
                if(execve("atomo", argv, envp) == -1) {
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
    /**todo: serve ancora?**/
   /* sleep(10);
*/
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
            execve("attivatore", argv, envp);
            if(execve("attivatore", argv, envp) == -1) {
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
            execve("alimentazione", argv, envp);
            if(execve("alimentazione", argv, envp) == -1) {
                perror("execve");
               	exit(EXIT_FAILURE);
            }
            exit(EXIT_FAILURE);
        default:
            break;
    }

    /*sleep(10);*/
    sem_down(semid);
    sem_wait_for_zero(semid);
    printf("START!\n");

    for(current_second = 0; current_second <= SIM_DURATION; current_second++) {
        printf("current_second: %d\n", current_second);
    }
    sleep(10);
    /*kill processi atomo*/
    /*for(i = 0; i < N_ATOMI_INIT; i++) {
        printf("atomi_pids[%d] = %d\n", i, atomi_pids[i]);
        kill(atomi_pids[i], SIGKILL);
    }*/

    /* Rimozione dei semafori e della memoria condivisa */
    semctl(semid, 0, IPC_RMID);
    semctl(semReadId, 0, IPC_RMID);
    shmctl(shmid, IPC_RMID, NULL);
    msgctl(msgid, IPC_RMID, NULL);

    printf("END!\n");

    return 0;
            
           

} 