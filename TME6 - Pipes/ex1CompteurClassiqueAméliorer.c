#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
int nbProcess = 0;
int nbProcessCreate = 0;
sem_t *barrier;
sem_t *compteur;
sem_t *mutex;

void wait_barrier(int NB_PCS){
	int i = 0;
	sem_wait(mutex);
	sem_wait(compteur);
	sem_getvalue(compteur,&i);
	if(i>0){
		//printf("%d | passage wait, %d \n",getpid(),nbProcessCreate);
		sem_post(mutex);
		sem_wait(barrier);
	}else {
		//printf("%d | dernier processus, %d \n",getpid(),nbProcessCreate);
		sem_post(mutex);
		for (i=1;i<nbProcess;i++) {
			//printf("Déblocage de i : %d\n",i);
			sem_post(barrier);
		}
		sem_close(barrier);
		sem_close(compteur);
		sem_close(mutex);
		sem_unlink("sem_toto_salleron");
		sem_unlink("sem_toto_compteur");
		sem_unlink("sem_toto_mutex");
		return;
	}
	sem_close(barrier);
	sem_close(compteur);
	sem_close(mutex);
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
	if((compteur = sem_open("sem_toto_compteur",O_CREAT|O_EXCL|O_RDWR,0666,nbProcess)) == SEM_FAILED){
			if(errno != EEXIST){
				perror("sem_open");
				exit(1);
			}
		}
		if((mutex = sem_open("sem_toto_mutex",O_CREAT|O_EXCL|O_RDWR,0666,nbProcess)) == SEM_FAILED){
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



