#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>

#define TOTO "sem_toto_1"
#define TITI "sem_titi_1"


int nbProcess = 0;
int nbProcessCreate = 0;
sem_t *barrier;
sem_t *mutex;

void wait_barrier(void * NB_PCS){
	int i = 0;
	sem_wait(mutex);
	(*(int *)NB_PCS)++;
	//printf("Compteur : %d \n",*ptr);
	if((*(int*)NB_PCS)<nbProcess){
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
		sem_close(mutex);
		sem_unlink(TOTO);
		sem_unlink(TITI);
		
		return;
	}
	sem_close(barrier);
	sem_close(mutex);
	return;
}

void process ( void * NB_PCS) {
	printf ("avant barrière\n");
	wait_barrier ((void *)NB_PCS);
	printf ("après barrière\n");
	exit (0);
}

int main(int argc, char *argv[]) {
	int *ptr = NULL;
	nbProcess = atoi(argv[1]);
	int i;
	int id;
	int res;
	//Ouverture
	if ((id = shm_open("monshm",O_RDWR | O_CREAT,0600)) == -1) {
		perror("Echec de l'allocation du descripteur\n");
		exit(errno);
	}

	/* Allocation pour stocker 1 entiers*/
	if(ftruncate(id,sizeof(int)) == -1){
		fprintf(stderr, "Echec d'allocation ftruncate\n");
	}
	/* Segmentation pour stocker 1 entiers*/
	if ((ptr = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, id, 0)) == MAP_FAILED){
		fprintf(stderr, "Echec de l'allocation de la ressource partagee\n");
		//exit(errno);
	}

	*ptr = 0;
	if((barrier = sem_open(TOTO, O_CREAT|O_EXCL|O_RDWR,0666,0)) == SEM_FAILED){
		if(errno != EEXIST){
			perror("sem_open");
			exit(1);
		}
	}
	if((mutex = sem_open(TITI,O_CREAT|O_EXCL|O_RDWR,0666,1)) == SEM_FAILED){
		if(errno != EEXIST){
			perror("sem_open");
			exit(1);
		}
	}

	for( i = 0; i<nbProcess;i++){
		if(fork() != 0)
			break;
	}

	printf("Valeur de cpt %d \n",*ptr);
	process((void *)ptr);

	/* Liberation de la ressource */
	munmap(&res, sizeof(int));

	if (shm_unlink("monshm")) {
			fprintf(stderr, "Ressource partagee mal rendue\n");
			exit(EXIT_FAILURE);
	}
}






	