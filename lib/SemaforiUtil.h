#include <sys/sem.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include "macro.h"
#ifndef PROGETTOSO_LIBSEMAFORI_H
#define PROGETTOSO_LIBSEMAFORI_H
/**
 * questa funzione inizializza il semaforo per la sincronizzazione iniziale
 * della simulazione, il valore del semaforo é passato come parametro
 * @param sem_id
 * @param val
 */
void sem_init(int sem_id, int val);
/**
 * questa funzione incrementa il valore del semaforo
 *in caso di errore stampa un msg
 * @param sem_id
 */
void sem_up(int sem_id);
/**
 * questa funzione decrementa il valore del semaforo
 * gestisce l'errore
 * OSS: cercare di decrementare un semaforo oltre lo zero è
 * una operazione bloccante
 * @param sem_id
 */
void sem_down(int sem_id);
/**
 * questa funzione fa sì che il processo si metta in attesa
 * fino a ché il semaforo non arriva 0
 * @param sem_id
 */
void sem_wait_for_zero(int sem_id);
#endif
