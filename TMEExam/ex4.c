#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>


//Ecrire un programme créant 4 Threads et attendant leur terminaison. Chaque Thread reçoit en argument son numéro de création (0, 1, 2 ou 3) et se contente de l’afficher. On souhaite cependant que l’ordre d’affichage soit toujours :
//
//	 Thread :0
//	Thread :1
//	Thread :2
//	Thread :3
//	CONTRAINTE : Pour synchroniser les affichages, vous ne pouvez utiliser que des variables de conditions et des variables Mutex, pas de signaux, sémaphores, fonction sleep, etc.
//
//	Exemple d'appel :
//	$PWD/bin/ordonner_legers

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int valCond = 0;

void *funcThread(void *arg){
	int numero = (*(int*)arg);
	srand((unsigned int )pthread_self());
	int status = numero + (int)(rand () /(((double) RAND_MAX +1) /2));
	printf("numero :%d, attente %d secondes\n",numero,status);
	sleep(status);
	pthread_mutex_lock(&mutex);
	while(valCond != numero)
		pthread_cond_wait(&cond, &mutex);
	valCond++;
	printf("Thread : %d\n",numero);
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);
	
	
	pthread_exit((void*)0);
}

int main(int argc, char *argv[]) {
	
	int nbThread = 10;
	pthread_t *tabThread = 0;
	tabThread = malloc(nbThread*sizeof(pthread_t));
	int i = 0;
	int *pt = 0;
	
	for(i = 0; i<nbThread ; i++){
		pt = malloc(sizeof(int));
		*pt = i;
		
		pthread_create(&tabThread[i],NULL,funcThread,pt);
	}
	
	for(i = 0; i<nbThread ; i++){
		pthread_join(tabThread[i],NULL);
	}
	
}

