#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "macro.h"


struct atomo {
    int numero_atomico;
    int pid;
    int stato; /* possiamo metterlo se serve tenere traccia degli atomi */
} atomo;

typedef struct message {
    long msg_type;
    char msg_text[2];
} message;

int getRandomNumber(int lowerLimit, int upperLimit);

void seedRandom();

int get_nano_sec();
