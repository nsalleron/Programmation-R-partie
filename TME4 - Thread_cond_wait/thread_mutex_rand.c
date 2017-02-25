#define _POSIX_SOURCE 1
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

//Modifier le programme de l’exercice précédent pour que chaque thread affiche non plus son numéro de création mais une valeur aléatoire entre 0 et 10. Pour cela, utilisez la fonction rand de la façon suivante :
//
//	(int) (10*((double)rand())/ RAND_MAX)
//
//	De plus, cette valeur aléatoire sera ajoutée à une variable globale, initialisée à zéro par le programme principal. On veillera évidemment à éviter les accès concurrents à cette variable, en utllisant les fonctions de la famille pthread_mutex_lock. Après terminaison de toutes les threads, le programme afficher la valeur finale de cette variable.
//
//	Exemple d'appel :
//	$PWD/bin/thread_rand 10

pthread_t *tab;
int val;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *funcThread(void *arg){
	pthread_mutex_lock(&mutex);
	free(arg);
	srand((unsigned int ) pthread_self());
	val += (int) (10*((double)rand())/ RAND_MAX);
	//val += *((int*)arg);	//(déférencement) (//Cast vers *entier) Pointeur
	printf("Valeur de val : %d\n",val);
	pthread_mutex_unlock(&mutex);
	pthread_exit((void*)0); return NULL;
}

int main(int argc, char *argv[]) {
	
	int i = 0;	
	int *pi;
	int *valeur;
	val  = 0;
	if(argc != 2)
		return EXIT_FAILURE;
		
	tab = malloc(atoi(argv[1])*sizeof(pthread_t));
		
	for (i =0;i<atoi(argv[1]);i++) {
		//passage de l'argument par le pointeur
		pi = malloc(sizeof(int));
		*pi = (int) (10*((double)rand())/ RAND_MAX);
		pthread_create(&tab[i],NULL,funcThread,pi);
	}
	for (i =0;i<atoi(argv[1]);i++) {
		if(pthread_join(tab[i],(void**)&valeur) !=0){
			printf("pthread_join\n");
			exit(1);	
		}
		//printf("La valeur de %d est %d\n",tab[i], valeur);
	}
	printf("Valeur finale : %d",val);

	
	
}