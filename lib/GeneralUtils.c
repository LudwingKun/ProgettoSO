
#include <stdlib.h>
#include "GeneralUtils.h"


void sem_init(int sem_id, int val) {
    if (semctl(sem_id, 0, SETVAL, val) == -1) {
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

void P(int send_id, int n, int sem_id) {
    struct sembuf op;
    op.sem_num = n;
    op.sem_op = -1;
    op.sem_flg = 0;
    semop(sem_id, &op, 1);
}

void V(int send_id, int n, int sem_id) {
    struct sembuf op;
    op.sem_num = n;
    op.sem_op = 1;
    op.sem_flg = 0;
    semop(sem_id, &op, 1);
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
