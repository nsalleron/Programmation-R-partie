#define _XOPEN_SOURCE 700
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include "libupper.h"


//On considère maintenant que le nombre de threads créées est inférieur au nombre de fichiers à traiter. Dès qu’une Thread a converti un fichier avec succès, elle doit passer à un autre fichier s’il en reste, et sinon se terminer. Si un fichier pose problème, elle se termine tout de suite en indiquant le fichier fautif au programme principal.

//Ecrire un nouveau programme C programmant cette stratégie, le premier argument sur la ligne de commande étant le nombre de Thread permis, les suivants étant les fichiers, en nombre supérieur. Le programme principal doit attendre la fin des Threads, tester leur retour et afficher les fichiers ayant posé problème. On notera qu’il peut y avoir des fichiers non examinés si toutes les Threads ont rencontré un fichier à problème.

//Nombre de fichiers restant
int *nbFichierRestant, *nbFichierMax;
//Nombre de threads
pthread_t* tabThread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



void *funcThread(void *arg){
	int *t = 0;
	t = malloc(sizeof(int));
	
	char ** arguments = (char**)arg;
	
	pthread_mutex_lock(&mutex);
	while (*nbFichierRestant>2) {	//Cad sans nom fichier et sans nb thread
		
		//printf("Valeur de nbFichierMax : %d et nbFichierRestant : %d\n",*nbFichierMax,*nbFichierRestant);
		printf("Thread : %ld - Traitement du fichier : %s\n",(long)pthread_self(),arguments[*nbFichierMax - *nbFichierRestant+2]);
		*nbFichierRestant = *nbFichierRestant - 1;
		//printf("Valeur de nbFichierMax : %d et nbFichierRestant : %d\n",*nbFichierMax,*nbFichierRestant);
		pthread_mutex_unlock(&mutex);
		
		//Traitement du fichier
		*t = upper(arguments[(*nbFichierMax - *nbFichierRestant)+1]);
		pthread_mutex_lock(&mutex);
		printf("Thread : %ld - Fin traitement valeur de t: %d\n",(long)pthread_self(),*t);
		
		//Vérification de la valeur de *t retour possible
		if(*t!=0){
			pthread_mutex_unlock(&mutex); //unlock car sortie
			printf("Thread : %ld - fin du Thread\n",(long)pthread_self());
			pthread_exit((void*)arguments[(*nbFichierMax - *nbFichierRestant)+1]);
		}
		
	}
	printf("Thread : %ld - fin du Thread correct\n",(long)pthread_self());
	
	*t = 0; //Dans ce cas tout est OK
	pthread_mutex_unlock(&mutex);
	pthread_exit((void*)t);
}





int main(int argc, char *argv[]) {
	
	int i;
	if(argc < 2)
		return EXIT_FAILURE;
	
	tabThread = malloc((argc-1)*sizeof(pthread_t));
	
	//On alloue l'emplacement du int dans le tas
	nbFichierMax = malloc((argc-1)*sizeof(int));
	nbFichierRestant = malloc((argc-1)*sizeof(int));
	
	*nbFichierMax = *nbFichierRestant = argc;
	
	for (i = 0;i<atoi(argv[1]);i++) {
		pthread_create(&tabThread[i], NULL, funcThread, (void *)argv); //On balance le tableau de char *
		printf("thread %d\n",i);
	}
	
	//Attente de l'ensemble des threads;
	void *valRet = 0;
	int valRetour = 0;
	for (i = 0;i<atoi(argv[1]);i++) {
			pthread_join(tabThread[i], (void**)&valRet);
			if((*(int*)valRet) != 0){
				printf("Le fichier %s pb \n",(char*)valRet);
				valRetour++;
			}
	}
	
	return valRetour;
	
	
}


