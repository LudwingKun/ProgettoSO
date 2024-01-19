#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	printf("Alimentazione creato!\n");
    printf("Alimentazione_pid: %d\n", getpid());
   	printf("padre alimentazione: %d\n", getppid());
}