
#include <stdlib.h>
#include "GeneralUtils.h"


void sem_init(int sem_id, int val, int n) {
    if (semctl(sem_id, n, SETVAL, val) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}

void sem_up(int sem_id) {
    struct sembuf sem_op = {0, 1, 0};
    if (semop(sem_id, &sem_op, 1) == -1) {
        perror("semop signal");
        exit(EXIT_FAILURE);
    }
}

void sem_down(int sem_id) {
    struct sembuf sem_op = {0, -1, 0};
    if (semop(sem_id, &sem_op, 1) == -1) {
        perror("semop wait");
        exit(EXIT_FAILURE);
    }
}

void sem_wait_for_zero(int sem_id) {
    struct sembuf sem_op = {0, 0, 0};
    if (semop(sem_id, &sem_op, 1) == -1) {
        perror("semop signal");
        exit(EXIT_FAILURE);
    }
}

void P(int n, int sem_id) {
    struct sembuf op;
    op.sem_num = n;
    op.sem_op = -1;
    op.sem_flg = 0;
    semop(sem_id, &op, 1);
    TEST_ERROR
}

void V(int n, int sem_id) {
    struct sembuf op;
    op.sem_num = n;
    op.sem_op = 1;
    op.sem_flg = 0;
    semop(sem_id, &op, 1);
    TEST_ERROR
}

void lettura_pipe(int fd[2], int *num) {
    close(fd[1]); /*chiusura lato scrittura*/
    read(fd[0], num, sizeof(int));
    close(fd[0]);
}

void scrittura_pipe(int fd[2], int *num) {
    close(fd[0]); /*chiusrua lato lettura*/
    write(fd[1], num, sizeof(int));

}


int random_btw(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void rimuovi_elemento(int array[], int *dimensione, int posizione) {
    if (posizione < 0 || posizione >= *dimensione) {
        printf("Posizione non valida\n");
        return;
    }
    for (int i = posizione; i < *dimensione; i++) {
        array[i] = array[i + 1];
    }
    (*dimensione)--;

}

int trova_pos(int array[], int *dimensione, pid_t pid) {
    int res;
    int i = 0;
    int non_trovato = 1;
    while (non_trovato && i < *dimensione) {
        if (array[i] == pid) {
            non_trovato = 0;
            res = i;
        }
        i++;
    }
    return res;
}

