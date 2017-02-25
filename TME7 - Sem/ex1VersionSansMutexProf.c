#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
int nbProcess = 0;
int nbProcessCreate = 0;
sem_t *barrier;

void wait_barrier(int NB_PCS){
	int i = 0;

	if(NB_PCS < nbProcess){
		//printf("%d | passage wait, %d \n",getpid(),nbProcessCreate);
		sem_wait(barrier);
	}else {
		//printf("%d | dernier processus, %d \n",getpid(),nbProcessCreate);
		for (i=1;i<nbProcess;i++) {
			//printf("Déblocage de i : %d\n",i);
			sem_post(barrier);
		}
		sem_close(barrier);
		sem_unlink("sem_toto_salleron");
		return;
	}
	sem_close(barrier);
	return;
}


void process (int NB_PCS) {
	printf ("avant barrière\n");
	wait_barrier (NB_PCS);
	printf ("après barrière\n");
	exit (0);
}

int main(int argc, char *argv[]) {

	nbProcess = atoi(argv[1]);
	int i;

	//printf("%d | nbProcess %d\n",getpid(),nbProcess);

	if((barrier = sem_open("sem_toto_salleron", O_CREAT|O_EXCL|O_RDWR,0666,0)) == SEM_FAILED){
		if(errno != EEXIST){
			perror("sem_open");
			exit(1);
		}
	}
	for( i = 0; i<nbProcess;i++){
		nbProcessCreate++;
		if(fork() != 0)
			break;
	}
	process(nbProcessCreate);
}
