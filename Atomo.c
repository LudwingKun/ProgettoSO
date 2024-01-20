#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
/*#include "macro.h"*/
#include "Util.h"

int main() {
    printf("Atomo di pid: %d\n\n", getpid());

    /*Primo atomo che trova prova a leggere dalla coda:*/
    int msgid;
    struct message myMessage;
    int current_second = 0;

    msgid = msgget(MSGKEY, 0);

/*    printf("msgid sono in atomo: %d\n",msgid);
    if (msgrcv(msgid, &myMessage, sizeof(myMessage), 1, 0) == -1) {
        perror("msgrcv yo");
        exit(EXIT_FAILURE);
    }*/


    while(current_second <= SIM_DURATION) {
        printf("cioa\n\n");
        if (current_second == STEP_ATTIVATORE) {
            msgrcv(msgid, &myMessage, sizeof(myMessage), 1, 0); /*1 sarebbe il tipo del messaggio */!
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


