#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#include <sys/msg.h>
#include <sys/errno.h>
#include <string.h>
#include "lib/macro.h"
#include "lib/Util.h"
#include "lib/MacroUtil.h"

/*Funziona però meglio se si trova una maniera migliore di strutturare il codice perchè sono un sacco di parametri!!*/

int main(int argc, char *argv[]) {
    /*Primo atomo che trova prova a leggere dalla coda:*/
    int msgid;
    struct message myMessage;
    int num_bytes;
    int STEP_ATTIVATORE, SIM_DURATION;
    int numero_atomico;

    if (argc != 4) {
        fprintf(stderr, "Error, wrong number of argument for atomo\n");
        exit(EXIT_FAILURE);
    }

    STEP_ATTIVATORE = atoi(argv[1]);
    SIM_DURATION = atoi(argv[2]);
    numero_atomico = atoi(argv[3]);
    printf("numero atomico dell'atomo %d e' ==== %d\n", getpid(), numero_atomico);


    msgid = msgget(MSGKEY, 0);
    if (msgid == -1) {
        fprintf(stderr, "%s: %d. Errore in msgget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("msgid sono in atomo: %d\n", msgid);

    while (1) {
        num_bytes = msgrcv(msgid, &myMessage, 100, getpid(), 0); /*1 sarebbe il tipo del messaggio */
        if(num_bytes == -1){
            if(errno == ENOMSG){
                printf("CHILD %d: nessun messaggio di quel tipo", getpid());
                sleep(2);
                continue;
            } else{
                printf("Errore n %d: %s", errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }else break;
    }
    printf("messaggio ricevuto per pid %d: %s\n\n",getpid(),  myMessage.msg_text);
}




/*msgid = msgget(MSGKEY, 0);
myMessage.msg_type = 1;
myMessage.msg_text = "scissione";
msgrcv(msgid, &myMessage, sizeof(myMessage), 1, 0);1 sarebbe il tipo del messaggio!

printf("messaggio ricevuto: %s\n\n", myMessage.msg_text); */


