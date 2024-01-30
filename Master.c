#include "lib/MacroUtil.h"
#include "lib/GeneralUtils.h"


int main() {

    /**************************DICHIARAZIONI DI VARIABILI QUI !!*************************************************/
    int i;
    int N_ATOMI_INIT, ENERGY_DEMAND, MIN_N_ATOMICO, N_NUOVI_ATOMI, N_ATOM_MAX, STEP_ALIMENTAZIONE, ENERGY_EXPLODE_THRESHOLD, SIM_DURATION, STEP_ATTIVATORE;
    int pid_attivatore, pid_alimentazione;
    int current_second;
    int shmidInit, semid, shmid_pids, msgid, shmid_dump;
    int *shmidInit_ptr;
    atomi *pids_ptr;
    dump *dump_ptr;
    int config = 0;
    char *configuration;

    int numero_atomico; /*num spedito dal padre*/
    int num; /*num letto dall'atomo*/
    int file_pipe[2];

    char buffer[10];
    char buffer2[10] = "dummy";

    char *args1[] = {"atomo", buffer, buffer2, NULL};
    char *args2[] = {"attivatore", buffer, NULL};
    char *args3[] = {"alimentazione", buffer, NULL};

    printf("Quale configurazione scegli? scrivi 1 per timeout, 2 per explode, 3 per blackout, 4 per meltdown\n");
    fscanf(stdin, "%d", &config);
    switch (config) {
        case 1:
            printf("Hai scelto la configurazione che termina per timeout\n");
            configuration = "config_timeout.txt";
            break;
        case 2:
            printf("Hai scelto la configurazione che termina per explode\n");
            configuration = "config_explode.txt";
            break;
        case 3:
            printf("Hai scelto la configurazione che termina per blackout\n");
            configuration = "config_blackout.txt";
            break;
        case 4:
            printf("Hai scelto la configurazione che termina per meltdown\n");
            configuration = "config_meltdown.txt";
            break;

    }

    /*************************************** LETTURA PARAMETRI *****************************************************/
    FILE *file = fopen((const char *) configuration, "r");

    if (file == NULL) {
        perror("Errore nell'apertura del file");
        exit(EXIT_FAILURE);
    }
    if (fscanf(file,
               "N_ATOMI_INIT=%d\nENERGY_DEMAND=%d\nN_ATOM_MAX=%d\nMIN_N_ATOMICO=%d\nSTEP_ATTIVATORE=%d\nSTEP_ALIMENTAZIONE=%d\nN_NUOVI_ATOMI=%d\nSIM_DURATION=%d\nENERGY_EXPLODE_THRESHOLD=%d\n",
               &N_ATOMI_INIT, &ENERGY_DEMAND, &N_ATOM_MAX, &MIN_N_ATOMICO, &STEP_ATTIVATORE, &STEP_ALIMENTAZIONE, &N_NUOVI_ATOMI, &SIM_DURATION, &ENERGY_EXPLODE_THRESHOLD) != 9) {
        fprintf(stderr, "Errore nella lettura dei parametri di configurazione\n");
        exit(EXIT_FAILURE);
    }

    fclose(file);
    /*************************************** FINE LETTURA PARAMETRI *****************************************************/



    /*************************************** CREAZIONE STRUTTURE *****************************************************/


    /*Creazione memoria condivisa per passaggio parametri + ids*/
    shmidInit = shmget(IPC_PRIVATE, sizeof(int) * (15) + 1, IPC_CREAT | 0666);
    if (shmidInit == -1) {
        fprintf(stderr, "%s: %d. Errore in shmget for shmidInit #%03d: %s\n", __FILE__, __LINE__, errno,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    /*Collegamento memoria condivisa*/
    shmidInit_ptr = (int *) shmat(shmidInit, NULL, 0);
    if (shmidInit_ptr == (void *) -1) {
        perror("shmat");
        return 1;
    }
/*TODO: CAMBIARE QUA**/
    /* Creazione MEMORIA CONDIVISA per array pid */
    shmid_pids = shmget(IPC_PRIVATE, sizeof(atomi), IPC_CREAT | 0666);
    if (shmid_pids == -1) {
        fprintf(stderr, "%s: %d. Errore in shmget for shmid_pids #%03d: %s\n", __FILE__, __LINE__, errno,
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("....shmid_pids sono in MASTER: %d\n", shmid_pids);

    /* Collega la memoria condivisa al puntatore*/
    pids_ptr = (atomi *) shmat(shmid_pids, NULL, 0);
    if (pids_ptr == (struct Atomi *) -1) {
        perror("shmat fail");
        TEST_ERROR
        return 1;
    }

    /* Creazione MEMORIA CONDIVISA per le statistiche ogni secondo, metto struttura DUMP! */
    shmid_dump = shmget(IPC_PRIVATE, sizeof(struct Dump), IPC_CREAT | 0666);
    if (shmid_dump == -1) {
        fprintf(stderr, "%s: %d. Errore in shmget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
/*dump_ptr è un puntatore alla memoria condivisa del dump*/
    dump_ptr = (struct Dump *) shmat(shmid_dump, NULL, 0);
    if ((struct Dump *) dump_ptr == (struct Dump *) (-1)) {
        perror("shmat fail");
        exit(EXIT_FAILURE);
    }

    /* creazione 8 semafori
     * 0)sincronizzazione processi iniziali
     * 1)attivazione
     * 2)scissione
     * 3)energia prodotta
     * 4)energia consumata
     * 5)scorie prodotta
     * 6)contatore Atomi
     * 7)controlla la struct Atomi*/

    semid = semget(IPC_PRIVATE, 8, IPC_CREAT | 0666);
    if (semid == -1) {
        fprintf(stderr, "%s: %d. Errore in semget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Creazione message queue */
    msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    if (msgid == -1) {
        fprintf(stderr, "%s: %d. Errore in msgget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }


    /*************************************** FINE CREAZIONE STRUTTURE *****************************************************/

    /*************************************** INIZIALIZZAZIONI *************************************************************/
    /* Fill memoria condivisa */
    shmidInit_ptr[0] = ENERGY_DEMAND;
    shmidInit_ptr[1] = MIN_N_ATOMICO;
    shmidInit_ptr[2] = ENERGY_EXPLODE_THRESHOLD;
    shmidInit_ptr[3] = STEP_ATTIVATORE;
    shmidInit_ptr[4] = SIM_DURATION;
    shmidInit_ptr[5] = STEP_ALIMENTAZIONE;
    shmidInit_ptr[6] = N_NUOVI_ATOMI;
    shmidInit_ptr[7] = N_ATOM_MAX;
    shmidInit_ptr[8] = shmid_pids;
    shmidInit_ptr[9] = shmid_dump;
    shmidInit_ptr[10] = semid;
    shmidInit_ptr[11] = msgid;


    if (sprintf((char *) args1[1], "%d", shmidInit) < 0) {
        printf("ERRORE");
    }

    /**seed initialize**/
    srand(time(NULL));


    /*Inizializzo semaforo per dump*/
    sem_init(semid, N_ATOMI_INIT + 1 + 1 + 1, 0);

    /*inizializzo il semaforo*/
    for (int j = 1; j < 8; j++) {
        sem_init(semid, 1, j);
    }

    /**init array pids**/
    /*P(6, semid);
    dump_ptr->contatore_atomi = 0;
    V(6, semid);*/

    /*P(7, semid);
    pids_ptr->indice = 0;
    pids_ptr->dimensione = 100;
    V(7, semid);*/
    /*************************************** FINE INIZIALIZZAZIONI *************************************************************/

    for (i = 0; i < N_ATOMI_INIT; i++) {
        if (pipe(file_pipe) == -1) {
            perror("fail in pipe");
            exit(EXIT_FAILURE);
        }
        switch (fork()) {
            case -1:
                perror("fork fail for atom");
                exit(EXIT_FAILURE);
            case 0:
                sem_down(semid); /*decremento del semaforo*/

                P(6, semid);
                dump_ptr->contatore_atomi++;
                V(6, semid);

                P(7, semid);
                pids_ptr->indice++;
                pids_ptr->pids[i] = getpid();
                printf("FIGLIO %d \n", getpid());
                V(7, semid);

                /*ricezione numero atomico*/
                close(file_pipe[1]); /*chiudi scrittura*/
                /*sleep(1);*/
                read(file_pipe[0], &num, sizeof(int));
                /*printf("numero letto: %d per atomo con pid: %d\n", num, getpid());*/
                close(file_pipe[0]);
                TEST_ERROR
                sprintf((char *) args1[2], "%d", num); /*conversione a stringa*/

                printf("!!!!LETTURA Atomo_pid %d da MASTER: %d\n", getpid(), getppid());

                if (execve("bin/atomo", args1, NULL) == -1) {
                    perror("execve");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_FAILURE);
            default:
                /* PIPE -> atomo si mette in ascolto e attende il numero atomico che gli invia il padre */
                close(file_pipe[0]);
                numero_atomico = random_btw(1, N_ATOM_MAX);
                /*printf("PADRE PRODUCE : %d \n", numero_atomico);*/
                write(file_pipe[1], &numero_atomico, sizeof(int));
                break;
        }

        close(file_pipe[1]);
    }


    pid_attivatore = fork();
    switch (pid_attivatore) {
        case -1:
            perror("error fork attivatore");
            exit(EXIT_FAILURE);
        case 0:
            sem_down(semid);

            printf("Attivatore con pid %d \n", getpid());

            if (execve("bin/attivatore", args2, NULL) == -1) {
                perror("execve");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_FAILURE);
        default:
            break;
    }


    pid_alimentazione = fork();
    switch (pid_alimentazione) {
        case -1:
            perror("error fork alimentazione");
            exit(EXIT_FAILURE);
        case 0:
            sem_down(semid);

            printf("Alimentazione con pid %d", getpid());

            if (execve("bin/alimentazione", args3, NULL) == -1) {
                perror("execve");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_FAILURE);
        default:
            break;
    }
    sleep(5);
    sem_down(semid);
    P(7, semid);
    for (int z = 0; z < 3; z++) {
        printf("YOYO: %d \n", pids_ptr->pids[z]);
    }
    V(7, semid);

    printf("Master  %d waiting for 0...\n", getpid());
    sem_wait_for_zero(semid);

    printf("||| ---------------------------------------------- INIZIO SIMULAZIONE --------------------------------------------------------- |||\n");

    int attivazioni = 0, scissioni = 0, en_prod = 0, en_cons = 0, scorie = 0, atomi = 0;

    for (current_second = 0; current_second <= SIM_DURATION ; current_second++) {
        P(1, semid);
        P(2, semid);
        P(3, semid);
        P(4, semid);
        P(5, semid);
        /*if ((dump_ptr->energia_prodotta - dump_ptr->energia_consumata) > ENERGY_EXPLODE_THRESHOLD) {*/
            /*************************** STATISTICHE (al sec) **********************************************************/
            printf("\n\n\n");
            printf("|-----------------------------------------------------------------------------------------------------------------------------|\n");
            printf("                                                 SECONDO %d\n", current_second);
            printf("|-----------------------------------------------------------------------------------------------------------------------------|\n");
            printf("|-----------------|---------------|----------------------|-------------------------|------------|--------------|--------------|\n");
            printf("|   ATTIVAZIONI   |   SCISSIONI   |   ENERGIA PRODOTTA   |    ENERGIA CONSUMATA    |   SCORIE   |              |              |\n");
            printf("|-----------------|---------------|----------------------|-------------------------|------------|--------------|--------------|\n");
            printf("|     %3d         |     %3d       |         %3d          |           %3d           |    %3d     |              |              |\n",
                   dump_ptr->attivazione - attivazioni,
                   dump_ptr->scissione - scissioni,
                   dump_ptr->energia_prodotta - en_prod,
                   dump_ptr->energia_consumata - en_cons,
                   dump_ptr->scoria_prodotta - scorie);
            /*pids_ptr->indice-atomi);*/
            printf("\n");
            printf("|-----------------|---------------|----------------------|-------------------------|------------|--------------|--------------|\n");
            printf("| ATTIVAZIONI TOT | SCISSIONI TOT | ENERGIA PRODOTTA TOT |  ENERGIA CONSUMATA TOT  | SCORIE TOT | ATOMI ATTIVI | ATOMI TOTALI |\n");
            printf("|-----------------|---------------|----------------------|-------------------------|------------|--------------|--------------|\n");
            printf("|     %3d         |     %3d       |         %3d          |           %3d           |    %3d     |      %3d     |      %3d     |\n",
                   dump_ptr->attivazione,
                   dump_ptr->scissione,
                   dump_ptr->energia_prodotta,
                   dump_ptr->energia_consumata,
                   dump_ptr->scoria_prodotta,
                   pids_ptr->indice,
                   dump_ptr->contatore_atomi);
            printf("|-----------------|---------------|----------------------|-------------------------|------------|--------------|--------------|\n");
            printf("\n");
            /**aggiornamento valori temporanei **/
            attivazioni = dump_ptr->attivazione;
            scissioni = dump_ptr->scissione;
            en_prod = dump_ptr->energia_prodotta;
            en_cons = dump_ptr->energia_consumata;
            scorie = dump_ptr->scoria_prodotta;
            atomi = pids_ptr->indice;
            V(1, semid);
            V(2, semid);
            V(3, semid);
            V(4, semid);
            V(5, semid);

            P(3, semid);
            printf("|-------------------------------------------------------CONSUMO ENERGIA %3d---------------------------------------------------|\n",
                   ENERGY_DEMAND);
            printf("\n\n\n");
            dump_ptr->energia_consumata += ENERGY_DEMAND;
            V(3, semid);
            sleep(1);
        /*}
        printf("");*/
    }
    /*************************** STATISTICHE (totali) **********************************************************/


    if (kill(pid_attivatore, SIGTERM) == 0) {
        printf("Segnale di terminazione inviato al processo con PID %d\n", pid_attivatore);
    } else {
        perror("Errore durante l'invio del segnale");
        return 1;
    }

    if (kill(pid_alimentazione, SIGTERM) == 0) {
        printf("Segnale di terminazione inviato al processo con PID %d\n", pid_alimentazione);
    } else {
        perror("Errore durante l'invio del segnale");
        return 1;
    }
    /**************/
    P(7, semid);
    for (int k = 0; k < pids_ptr->indice; k++) {
        if (kill(pids_ptr->pids[k], SIGTERM) == 0) {
        } else {
            perror("Errore durante l'invio del segnale");
            return 1;
        }
    }
    V(7, semid);
    sleep(5);

    /*Questa sleep serve sennò il master finisce prima degli altri processi*/



    /*------------------Rimozione dei semafori e della memoria condivisa---------------------------- */

    /*detach memoria condivisa per array pid*/
    if (shmdt(pids_ptr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    /* detach memoria condivisa dal processo per dump */
    if (shmdt(dump_ptr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    /* detach memoria condivisa dal processo per Init */
    if (shmdt(shmidInit_ptr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    /* rimozione memoria condivisa per dump */
    if (shmctl(shmid_dump, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }

    for (int (n) = 0; (n) < 8; ++(n)) {

        semctl(semid, n, IPC_RMID);
    }

    shmctl(shmid_pids, IPC_RMID, NULL);
    shmctl(shmidInit, IPC_RMID, NULL);
    msgctl(msgid, IPC_RMID, NULL);

    printf("La simulazione e' terminata perche' sono trascorsi %d secondi\n", SIM_DURATION);

    printf("\n||| ------------------------------------------------FINE SIMULAZIONE--------------------------------------------------------- |||\n");

    return 0;


}
