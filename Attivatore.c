#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/errno.h>
#include <string.h>
#include "lib/macro.h"
#include "lib/Util.h"
#include <sys/shm.h>



int main(int argc, char *argv[]) {
    int msgid, shmid,shmidReport,i;
    struct message myMessage;
    int current_second = 0;
    int STEP_ATTIVATORE, SIM_DURATION;
    int *puntatore_pids;
    struct Dump *shared_dump;
    int num_bytes;

    if (argc != 4) {
        fprintf(stderr, "Error, wrong number of argument for attivatore\n");
        exit(EXIT_FAILURE);
    }


    STEP_ATTIVATORE = atoi(argv[1]);
    SIM_DURATION = atoi(argv[2]);
    shmid=atoi(argv[3]);
    /*shmidReport=atoi(argv[4]);*/

 /*Collega la memoria condivisa Report al puntatore*/
    /*shared_dump = (struct Dump *) shmat(shmidReport, NULL, 0);
    if ((struct Dump *) shared_dump == (struct Dump *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }*/

    /* Collega la memoria condivisa pids al puntatore*/
    puntatore_pids = (int*)shmat(shmid,NULL,0);
    if (puntatore_pids == (void *)-1) {
        perror("shmat");
        return 1;
    }

    printf("!!!!!LETTURA pid da ATTIVATORE: %d\n",puntatore_pids[0]);
    printf("!!!!!LETTURA pid da ATTIVATORE: %d\n",puntatore_pids[1]);
    printf("!!!!!LETTURA pid da ATTIVATORE: %d\n",puntatore_pids[2]);

    msgid = msgget(MSGKEY, 0);
    if (msgid == -1) {
        fprintf(stderr, "%s: %d. Errore in msgget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("msgid sono in ATTIVATORE: %d\n",msgid);
    /*TODO: FALSO COME GIUDA QUEL 3, LÌ CI VA IL CONTATORE DEI PID*/
    while(puntatore_pids[i]!=NULL){
        myMessage.msg_type = puntatore_pids[i];
        num_bytes = sprintf(myMessage.msg_text, "scissione");
        msgsnd(msgid, &myMessage, num_bytes, 0);
        i++;
        /*shared_dump->attivazione++;*/
    }



/*
    while(current_second <= SIM_DURATION) {
        if (current_second == STEP_ATTIVATORE) {

        }
        current_second++;
    }
    */
    }




/*Capire questione del tempo, tutto qui commentato:*/
/*
int get_nano_sec() {
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME, &ts) == -1) {
		perror("clock_gettime");
		exit(1);
	}
	return ts.tv_nsec;
}

int get_sec() {
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME, &ts) == -1) {
		perror("clock_gettime");
		exit(1);
	}
	return ts.tv_sec;
}

int get_simulated_time() {
	static int simulated_time = 0;
	return simulated_time;
}

void activate_atoms(int msgid) {
	int activation_time = get_nano_sec();

	if (get_nano_sec() - activation_time == STEP_ATTIVATORE) {
		send_message(msgid);
		activation_time = get_nano_sec();
	}
}

int main() {
	printf("Attivatore creato!\n");
   	printf("Attivatore_pid: %d\n", getpid());
    printf("padre attivatore: %d\n", getppid());


	int simulated_time_step = 1;
	int activation_time = 5;
	int messsage_atom = 0;
	int msgid;
    struct message myMessage;
    int simulated_time;

	msgid = msgget(MSGKEY, 0);
    myMessage.msg_type = 1;
    myMessage.msg_text = "scissione";

    printf("Get simulated time: %d\n", get_simulated_time());

	while (get_simulated_time() < SIM_DURATION) {
		if (STEP_ATTIVATORE >= get_simulated_time()) {
			msgsnd(msgid, &myMessage, sizeof(myMessage), 0);
			printf("entra qui?");
			messsage_atom = 1;
		}
		simulated_time += simulated_time_step;

		struct timespec ts_sleep;
		ts_sleep.tv_sec = simulated_time_step;
		ts_sleep.tv_nsec = 0;

		nanosleep(&ts_sleep, NULL);

	}



    int i;


    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    printf("time: %d\n", clock_gettime(CLOCK_REALTIME, &current_time));

    double elapsed_time = difftime(current_time.tv_sec, TIME);
    printf("elapsed_time: %f\n\n", elapsed_time);


    printf("STEP_ATTIVATORE: %d\n", STEP_ATTIVATORE);
    printf("activation_time: %d\n", &activation_time);

    if(elapsed_time == STEP_ATTIVATORE) {
    	printf("entra qui?");
    }

	if ( == STEP_ATTIVATORE) {
		for (i = 0; i < N_ATOMI_INIT; i++) {
			if (msgsnd(msgid, &myMessage, sizeof(myMessage), 0)) {
				activation_time = get_nano_sec();
				printf("entra qui?");
			}
		}
	}

} */
