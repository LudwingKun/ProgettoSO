#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "macro.h"

struct Atomo {
	int numero_atomico;
	int pid;
	int stato; 				/* possiamo metterlo se serve tenere traccia degli atomi */
}atomo;

typedef struct message{
	long msg_type;
	char msg_text[2];
}message;

/*Dump per riportare le statistiche ogni secondo*/
typedef struct Dump{
    int attivazione;
    int scissione;
    int energia_prodotta;
    int energia_consumata;
    int scoria_prodotta;
}dump;

/*vediamo se serve anche quello totale*/

int getRandomNumber(int lowerLimit, int upperLimit);
void seedRandom();
int get_nano_sec();
void setMacro();