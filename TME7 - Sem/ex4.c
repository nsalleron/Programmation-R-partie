#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
int nbProcess = 0;
int nbProcessCreate = 0;
sem_t *sem;

//Nous considérons qu’un processus crée N processus fils. Chaque processus fils possède un identifiant unique (1, …, N). 
//Chacun commence par s’endormir pendant (N % identifiant) secondes. 
//A son réveil il affiche son identifiant et son Pid, cependant ces affichages doivent être faits par ordre croissant d’identifiant. 
//Pour assurer une telle synchronisation, vous devrez utiliser des sémaphores anonymes.

//Le processus père doit attendre la terminaison de tous ses fils avant de se terminer lui aussi.

void process(int i ){
	//printf("Valeur de i : %d\n",i);
	sleep(i%getpid());
	if(i>0)
		sem_wait(&sem[i-1]);
	printf("PID : %d identifiant : %d\n",getpid(),i);
	if(i<nbProcess-1)
		sem_post(&sem[i]);
	
}


int main(int argc, char *argv[]) {

	nbProcess = atoi(argv[1]);
	printf("Père : %d\n",getpid());
	int i;
	sem = malloc(sizeof(sem_t)*nbProcess);
	
	
	for( i = 0; i<nbProcess-1;i++){
		sem_init(&sem[i], 0, 0);
	}
	
	
	nbProcessCreate = nbProcess;
	for( i = 0; i<nbProcess-1;i++){
		nbProcessCreate--;
		if(fork() != 0)
			break;
	}
	process(i);
	
	wait(NULL);
	
	

}