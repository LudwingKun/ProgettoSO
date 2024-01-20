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
2 Memoria Condivisa dove inseriamo le struct di tutti gli atomi creati
Struct atomo -> pid, numeroAtomico

1 Coda di Messaggi -> Attivatore invia segnale di Scissione all'atomo