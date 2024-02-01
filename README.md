# ProgettoSO

Semaforo iniziale per la sincronizzazione dei processi così che la simulazione inizi dopo che tutti i processi sono stati
creati e le strutture create. 

------------------------DUMP---------------------------
1 Memoria Condivisa che tiene traccia di:
- numero di attivazioni dell'attivatore -> attivatore scrive
- numero di scissioni totali -> 
- numero di scissioni ogni secondo 
- quantità di energia prodotta totale
- quantità di energia prodotta ogni secondo
- energia consumata totale
- energia consumata ogni secondo
(30)
- quantita' di energia assorbita dal processo inibitore
- log delle operazioni di bilanciamento condotte dal processo inibitore

master legge e stampa a video i dati della memoria condivisa
TODO: master ogni secondo stampa con un segnale (anche zero all'inizio)

--------------------------ATOMI------------------------
2 Memoria Condivisa dove inseriamo le struct di tutti gli Atomi creati
Struct atomo -> pid, numeroAtomico

1 Coda di Messaggi -> Attivatore invia segnale di Scissione all'atomo

MEMORIA CONDIVISA dove inseriamo i parametri, id memoria condivisa, coda di messaggi, semaforo
9 parametri + idsemaforo + idcoda + idmemoriaPid + idmemoriaCondivisaDump


## Relazione progetto di Sistemi Operativi 2023/2024

## Componenti del gruppo:
- 1
- 2
- 3

Il progetto è composto da 4 processi principali:
- Master
- Atomo
- Alimentazione
- Attivatore

.
.
.


## Master
Il processo master gestisce i ritmi delle simulazioni, procede subito alla lettura dei parametri delle configurazioni 
contenuti nei file .txt allegati, successivamente crea una memoria condivisa che conterrà:
i valori iniziali della simulazione, gli id del set di semafori, della coda di messaggi e della memoria condivisa 
utilizzata per la raccolta dei dati utili alle stampe della simulazione.
Successivamente si occupa della creazione iniziale degli altri tre processi attraverso una fork() che al suo interno,
tramite una execve(), passa il codice contenuto nei file .c dei rispettivi processi e il buffer con l'indirizzo alla 
memoria condivisa contenente gli id elencati in precedenza, utili per le varie attività dei processi.
Attesa la corretta inizializzazione degli altri processi creati attraverso un semaforo (quando questo raggiunge lo zero)
inizia l'effettiva simulazione stampando ogni secondo i valori dei dati richiesti presenti in memoria condivisa, sempre
grazie all'utiizzo di semafori per la loro lettura.
Ad ogni ciclo di stampa controlla che non si sia verificato nessun caso di "terminazione", se così fosse procede a 
mandare segnali ai processi per i vari detach dalle strutture condivise, successivamente anche il master procede al detach,
andando tutto a buon fine stampa la motivazione della fine della simulazione e quest'ultima termina.  