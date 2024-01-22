CFLAGS = -std=c89 -pedantic -Wall

all: master atomo attivatore alimentazione

clean:
	rm -f *.o bin/master bin/atomo bin/attivatore bin/alimentazione *~

master: Master.c Makefile
	gcc $(CFLAGS) Master.c lib/GeneralUtils.c  -o bin/master

atomo: Atomo.c Makefile
	gcc $(CFLAGS) Atomo.c -o bin/atomo

attivatore: Attivatore.c Makefile
	gcc $(CFLAGS) Attivatore.c -o bin/attivatore

alimentazione: Alimentazione.c Makefile
	gcc $(CFLAGS) Alimentazione.c -o bin/alimentazione

run:  all
	./bin/master
start:
	./bin/master