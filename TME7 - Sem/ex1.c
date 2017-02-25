#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int nbProcess = 0;
sem_t *barrier;

void wait_barrier(int NB_PCS){
	int i = 0;
	if(NB_PCS != nbProcess)
		sem_wait(barrier);
	else {
		for (i=0;i<nbProcess;i++) {
			printf("Déblocage de i : %d",i);
			sem_post(barrier);
		}
		
	}
	sem_close(barrier);
}


void process (int NB_PCS) {
	printf ("avant barrière");
	wait_barrier (NB_PCS);
	printf ("après barrière");
	exit (0);
}

int main(int argc, char *argv[]) {
	
	nbProcess = atoi(argv[1]);
	int i;

	
	if((barrier = sem_open("/sem", O_CREAT|O_EXCL|O_RDWR,0666,0))){
		if(errno != EEXIST){
			perror("sem_open");
			exit(1);
		}
	}
	
	for( i = 0; i<nbProcess;i++){
		if(fork() != 0)
			break;
	}	
	printf("Valeur de i = %d\n",i);
	process(i);
	
	
	
	
}