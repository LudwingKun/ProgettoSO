#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "macro.h"
#include "util.h"


int get_nano_sec() {
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME, &ts) == -1) {
		perror("clock_gettime");
		exit(1);
	}
	return ts.tv_nsec;
}

/*void activate_atoms(int msgid) {
	int activation_time = get_nano_sec();

	if (get_nano_sec() - activation_time == STEP_ATTIVATORE) {
		send_message(msgid);
		activation_time = get_nano_sec();
	}
}*/

int main() {
	printf("Attivatore creato!\n");
   	printf("Attivatore_pid: %d\n", getpid());
    printf("padre attivatore: %d\n", getppid());


    int activation_time = get_nano_sec();
    int i;
    int msgid;
    struct message myMessage;

    msgid = msgget(MSGKEY, 0);
    myMessage.msg_type = 1;
    myMessage.msg_text = "scissione";


	if (get_nano_sec() - activation_time == STEP_ATTIVATORE) {
		for (i = 0; i < N_ATOMI_INIT; i++) {
			if (msgsnd(msgid, &myMessage, sizeof(myMessage), 0)) {
				activation_time = get_nano_sec();
			}
		}
	}
    
}