#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/errno.h>
#include <string.h>
#include "lib/macro.h"
#include "lib/Util.h"


/*Funziona però meglio se si trova una maniera migliore di strutturare il codice perchè sono un sacco di parametri!!*/

int main(int argc, char *argv[]) {
    /*Primo atomo che trova prova a leggere dalla coda:*/
    int msgid;
    struct message myMessage;
    int current_second = 0;
    int STEP_ATTIVATORE, SIM_DURATION;

    if (argc != 3) {
        fprintf(stderr, "Error, wrong number of argument for atomo\n");
        exit(EXIT_FAILURE);
    }

    STEP_ATTIVATORE = atoi(argv[1]);
    SIM_DURATION = atoi(argv[2]);

    msgid = msgget(MSGKEY, 0);
    if (msgid == -1) {
        fprintf(stderr, "%s: %d. Errore in msgget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("msgid sono in atomo: %d\n", msgid);

/***TODO: cosa è quel atomo?
 *
 *dichiarato ****/
    /*Prendere da memoria condivisa?*/
    printf("numero atomico dell'atomo %d e' ==== %d\n", atomo.pid, atomo.numero_atomico);
    while(current_second <= SIM_DURATION) {
        printf("cioa\n\n");
        if (current_second == STEP_ATTIVATORE) {
            msgrcv(msgid, &myMessage, sizeof(myMessage), 1, 0); /*1 sarebbe il tipo del messaggio */
            printf("messaggio ricevuto: %s\n\n", myMessage.msg_text);
        }
        current_second++;
    }
}




    /*msgid = msgget(MSGKEY, 0);
    myMessage.msg_type = 1;
    myMessage.msg_text = "scissione";
	msgrcv(msgid, &myMessage, sizeof(myMessage), 1, 0);1 sarebbe il tipo del messaggio!

	printf("messaggio ricevuto: %s\n\n", myMessage.msg_text); */


