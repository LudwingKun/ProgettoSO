#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "lib/Util.h"

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


int main(int argc, char *argv[]) {
    int i;
    int atomi_alimentatore[10];
    int current_second = 0;
    int SIM_DURATION, STEP_ALIMENTAZIONE, N_NUOVI_ATOMI, N_ATOM_MAX;

    if (argc != 5) {
        fprintf(stderr, "Error, wrong number of argument for alimentazione\n");
        exit(EXIT_FAILURE);
    }

    SIM_DURATION = atoi(argv[1]);
    STEP_ALIMENTAZIONE = atoi(argv[2]);
    N_NUOVI_ATOMI = atoi(argv[3]);
    N_ATOM_MAX = atoi(argv[4]);

	printf("Sono dentro Alimentazione\n");
    printf("pid alimentazione! %d\n", getpid());

  /*  while(current_second <= SIM_DURATION) {
        if (current_second == STEP_ALIMENTAZIONE) {
            printf("Alimentazione deve creare atomi!\n\n\n");
            for (i = 0; i < N_NUOVI_ATOMI; i++) {
                atomi_alimentatore[i] = fork();
                switch (atomi_alimentatore[i]) {
                    case -1:
                        perror("fork");
                        exit(EXIT_FAILURE);
                    case 0:
                        printf("Atomo da Alimentatore creato!\n");
                        printf("Atomo_Alimentatore_pid: %d\n", getpid());
                        printf("padre atomo creato da Alimentatore: %d\n", getppid());
                        *//*execve("atomo",args1 , NULL);
                        if(execve("atomo", args1, NULL) == -1) {
                            perror("execve");
                            exit(EXIT_FAILURE);
                        }*//*
                        exit(EXIT_FAILURE);
                    default:
                        break;
                }
            }

        }
        current_second++;
    }*/

}