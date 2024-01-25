#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/shm.h>
#include "lib/Util.h"

void seedRandom(void) {
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

/**TODO: da mettere in memoria condivisa i pid degli atomi che crea, serve un semaforo? */


int main(int argc, char *argv[]) {
    int i;
    int atomi_alimentatore[10];
    int current_second;
    int SIM_DURATION, STEP_ALIMENTAZIONE, N_NUOVI_ATOMI, N_ATOM_MAX;
    char buffer[10];
    int numero_atomico; /*num spedito dal padre*/
    int num; /*num letto dall'atomo*/
    int file_pipe[2];
    int shmidInit;
    int *shmidInit_ptr;


    shmidInit = atoi(argv[1]);

    /*Collega la memoria condivisa Init al puntatore*/
    shmidInit_ptr = (int *) shmat(shmidInit, NULL, 0);
    if ((int *) shmidInit_ptr == (void *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }

    STEP_ALIMENTAZIONE = shmidInit_ptr[5];
    printf("STEP_ALIMENTAZIONE: %d\n", STEP_ALIMENTAZIONE);
    SIM_DURATION = shmidInit_ptr[4];
    N_NUOVI_ATOMI = shmidInit_ptr[6];
    N_ATOM_MAX = shmidInit_ptr[7];


    /* detach memoria condivisa per init */
    if (shmdt(shmidInit_ptr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }



    /*char *args1[] = {"atomo", buffer, NULL};*/

	printf("Sono dentro Alimentazione\n");
    printf("pid alimentazione! %d\n", getpid());

    current_second = 0;
    while(current_second <= SIM_DURATION) {
        if (current_second == STEP_ALIMENTAZIONE) {
            printf("Alimentazione deve creare atomi! sono al secondo: %d\n\n\n", current_second);
            for (i = 0; i < N_NUOVI_ATOMI; i++) {
                if(pipe(file_pipe) == -1){
                    perror("fail in pipe");
                    exit(EXIT_FAILURE);
                }
                atomi_alimentatore[i] = fork();
                switch (atomi_alimentatore[i]) {
                    case -1:
                        perror("fork");
                        exit(EXIT_FAILURE);
                    case 0:
                        printf("Atomo_Alimentatore_pid: %d\n", getpid());
                        /*ricezione numero atomico*/
                        close(file_pipe[1]); /*chiudi scrittura*/
                        sleep(1);
                        read(file_pipe[0], &num, sizeof(int));
                        printf("numero letto: %d per atomo con pid: %d\n", num, getpid());
                        close(file_pipe[0]);
                        /*TEST_ERROR*/
                        /*sprintf(args1[1], "%d", num); /*conversione a stringa
                        printf("!!!!LETTURA Atomo_pid da ALIMENTAZIONE: %d\n", getpid());
                        if (execve("bin/atomo", args1, NULL) == -1) {
                            perror("execve");
                            exit(EXIT_FAILURE);
                        }*/
                        printf("Numero Atomico dell'atomo con pid %d e': %d\n", getpid(),
                               getRandomNumber(1, N_ATOM_MAX));
                        exit(EXIT_FAILURE);
                    default:
                        /* PIPE -> atomo si mette in ascolto e attende il numero atomico che gli invia il padre */
                        close(file_pipe[0]);
                        numero_atomico = getRandomNumber(1, N_ATOM_MAX);
                        printf("PADRE ALIMENTAZIONE PRODUCE : %d \n", numero_atomico);
                        write(file_pipe[1], &numero_atomico, sizeof(int));
                        break;
                }
            }
            close(file_pipe[1]);
            STEP_ALIMENTAZIONE = STEP_ALIMENTAZIONE*2;
        }
        current_second++;
    }

}
