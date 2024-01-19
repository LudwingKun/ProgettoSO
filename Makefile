CFLAGS = -std=c89 -pedantic

all: master atomo attivatore alimentazione

clean:
	rm -f *.o master atomo attivatore alimentazione *~

master: Master.c Makefile
	gcc $(CFLAGS) Master.c -o master

atomo: Atomo.c Makefile
	gcc $(CFLAGS) Atomo.c -o atomo

attivatore: Attivatore.c Makefile
	gcc $(CFLAGS) Attivatore.c -o attivatore

alimentazione: Alimentazione.c Makefile
	gcc $(CFLAGS) Alimentazione.c -o alimentazione
	

run:  all
	./master
