#include <sys/sem.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "MacroUtil.h"

#ifndef PROGETTOSO_LIBSEMAFORI_H
#define PROGETTOSO_LIBSEMAFORI_H
                /*************************************SEMAFORI***********************************************/

/**
 * questa funzione inizializza il semaforo per la sincronizzazione iniziale
 * della simulazione, il valore del semaforo é passato come parametro
 * @param sem_id
 * @param val
 */
void sem_init(int sem_id, int val, int n);

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
/**
 * funzione che svolge le operazioni di lettura da una pipe
 * e salva il contenuto nella variabile intera num
 * @param fd
 * @param num
 */
void lettura_pipe(int fd[2], int *num);
/**
 * scrive sulla pipe l'intero passatogli
 * @param fd
 * @param num
 */
void scrittura_pipe(int fd[2], int *num);

void P(int n, int sem_id);

void V(int n, int sem_id);

/**
 *
 * @param min
 * @param max
 * @return numero intro random tra min e max inclusi
 */
int random_btw(int min, int max);
/**
 * la funzione rimuove, facendo uso di un semaforo, l'elemento
 * alla posizione indicata e riscala il vettore
 * @param array
 * @param dimensione
 * @param posizione
 */
void rimuovi_elemento(int array[], int *dimensione, int posizione);
/**
 * la funzione trova la posizione del pid corrispondente
 * @param array
 * @param dimensione
 * @param pid
 * @return posizione corrispondente
 */
int trova_pos(int array[], int *dimensione, pid_t pid);


void gestore_segnale();


/*Dump per riportare le statistiche ogni secondo*/
typedef struct Dump{
    int attivazione;
    int scissione;
    int energia_prodotta;
    int energia_consumata;
    int scoria_prodotta;
    int contatore_atomi;
}dump;

/*vediamo se serve anche quello totale*/

typedef struct message {
    long msg_type;
    char msg_text[100];
} message;

void seedRandom(void);

#endif
