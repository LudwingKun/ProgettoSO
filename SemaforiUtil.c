#include "SemaforiUtil.h"

void sem_init(int sem_id){
    if (semctl(sem_id, 0, SETVAL, N_ALIMENTAZIONE + N_ATOMI_INIT + N_ATTIVATORE +1) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}
void sem_up(int sem_id){
    struct sembuf sem_op = {0, 1, 0};
    if (semop(sem_id, &sem_op, 1) == -1) {
        perror("semop signal");
        exit(EXIT_FAILURE);
    }
}
void sem_down(int sem_id){
    struct sembuf sem_op = {0, -1, 0};
    if (semop(sem_id, &sem_op, 1) == -1) {
        perror("semop wait");
        exit(EXIT_FAILURE);
    }
}
void sem_wait_for_zero(int sem_id){
    struct  sembuf sem_op = {0, 0, 0};
    if(semop(sem_id, &sem_op, 1) ==-1){
        perror("semop signal");
        exit(EXIT_FAILURE);
    }
}