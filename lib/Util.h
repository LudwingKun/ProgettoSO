#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "macro.h"


struct Atomo {
	int numero_atomico;
	int pid;
	int stato; 				/* possiamo metterlo se serve tenere traccia degli atomi */
}atomo;


typedef struct message {
    long msg_type;
    char msg_text[100];
} message;

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


int random_btw(int min, int max);
void seedRandom(void);



