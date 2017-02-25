#define _XOPEN_SOURCE 700
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include "libupper.h"
#include <signal.h>
//On désire créer une chaîne de N threads (la Thread principale crée une Thread, qui à son tour en crée une autre, et ainsi de suite N fois) qui fonctionne de la manière suivante. 	OK

//Au démarrage du programme, la Thread principale masque tous les signaux, démarre la chaîne de création puis attend que toutes les Threads soient créées avant d’afficher "Tous mes descendants sont créés".	OK

//Après leur création, toutes les Threads sauf la principale se bloquent en attendant que celle-ci les libère. Parallèlement, la Thread principale se bloque en attente d’un signal SIGINT émis par l’utilisateur avec un CTRL+C.

// A la délivrance de ce signal, elle se débloque et débloque les autres Threads de la chaîne puis attend enfin que toutes se soient terminées avant d’afficher "Tous mes descendants se sont terminés".

//NB : Seule la Thread principale doit pouvoir être interrompue par un signal. On rappelle que chaque Thread gère un masque de signaux qui lui est propre.
int threadCreation = 1;
int signalPasRecu = 1;
pthread_mutex_t threadCrea = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condCreation = PTHREAD_COND_INITIALIZER;
pthread_t *tabThread;

void *funcThread(void* arg){
	
	pthread_mutex_lock(&threadCrea);
	printf("Valeurs de pMaxThread : %d\n",(*(int*)arg));
	
	if((*(int*)arg>0)){ //On doit encore faire N Thread;
		(*(int*)arg)--;
		pthread_create(&tabThread[(*(int*)arg)], NULL, funcThread, (void*)arg);
	}else{	//On est au dernier Thread
		
		printf("Passage dans la fin \n");
		pthread_cond_broadcast(&condCreation);
		threadCreation = 0;
		
	}
	
	printf("Attente de la création de toutes les threads \n");
	/* ------ Attente de la création de toutes les threads ------*/
	while(threadCreation){
		pthread_cond_wait(&condCreation, &threadCrea);
	}
	
	//printf("Attente du dévérouillage par le thread 0 \n");
	/* ------ Attente du dévérouillage par le Thread 0 (main) ------*/
	while(signalPasRecu){
		pthread_cond_wait(&condCreation, &threadCrea);
	}
	pthread_mutex_unlock(&threadCrea);
	pthread_exit((void*)0);
}


int main(int argc, char *argv[]) {
	int i = 0;
	int maxThread;
	int *pmaxThread;
	
	/* ------ Vérification des arguments ------*/
	if(argc<2)
		return EXIT_FAILURE;
	
	/* ------ Masquage des signaux ------*/
	sigset_t ens; int sig;
	sigfillset(&ens);
	pthread_sigmask(SIG_SETMASK,&ens,NULL);
		
	/* ------ Récupération nombre chaine thread ------*/
	maxThread = atoi(argv[1]);
	pmaxThread = malloc(sizeof(int));
	*pmaxThread = maxThread-1;
	
	tabThread = malloc(maxThread * sizeof(pthread_t));
	/* ------ Creation chaine de pmaxThread ------*/
	pthread_create(&tabThread[maxThread], NULL, funcThread, (void*)pmaxThread);
	
	/* ------ Attente fin creation thread et message ------*/
	pthread_mutex_lock(&threadCrea);
	while(threadCreation){
		pthread_cond_wait(&condCreation, &threadCrea);
	}
	
	pthread_mutex_unlock(&threadCrea);
	printf("Tous mes descandants sont crées.\n ");
	
	/* ------ Attente signal user ------*/
	sigemptyset(&ens);
	sigaddset(&ens, SIGINT);
	int valReturn = 0;
	while(1){
		valReturn = 0;
		sigwait(&ens, &valReturn);
		/* ------ Réveil de l'ensemble des Threads ------*/
		if(valReturn == SIGINT){
			pthread_cond_broadcast(&condCreation);
			signalPasRecu = 0;
			break;
		}

	}
		
	/* ------ Attente de l'ensemble des Threads ------*/
	for(i = 0; i<maxThread;i++){
		pthread_join(tabThread[i], NULL);
	}
	printf("Tous mes descandants sont fini.\n ");
	
}