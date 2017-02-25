#define _POSIX_SOURCE 1
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>


//Modifier le programme précédent pour que la valeur finale soit affichée non plus par le programme principal mais par une nouvelle thread créée au départ. Celle-ci, après sa création, doit se bloquer en attendant que la somme de toutes les valeurs aléatoires soit complétée. 
//La dernière thread à ajouter sa valeur aléatoire utilisera pthread_cond_signal pour signifier à la première qu’elle peut afficher la valeur de la globale. 

//A VOIR CETTE PARTIE
//On utilisera une variable statique dans la fonction appelée à la création de la thread pour compter le nombre de ses appels, et repérer ainsi le dernier appel (le problème peut se résoudre sans d’autres variables globales que celle additionnant les valeurs aléatoires).
//
//Exemple d'appel :
//$PWD/bin/thread_wait 10


pthread_t *tab;
int valeur = 0;
int val_thread_max = 0;
int val_thread_courant = 0;
int condition =0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *funcThread(void *arg){
	pthread_mutex_lock(&mutex);
	val_thread_courant++;
	valeur += (int) (10*((double)rand())/ RAND_MAX);
	printf("Valeur de val : %d\n",valeur);
	if(val_thread_courant == val_thread_max){	//On est dans le dernier Thread;
		condition = 1;
		pthread_cond_signal(&cond);
	}
	pthread_mutex_unlock(&mutex);
	pthread_exit((void*)0); return NULL;
}

void *funcAttente(void *arg){
	pthread_mutex_lock(&mutex);
	//ici il n'y a qu'un seul thread qui doit attendre.
	while (!condition) {
		pthread_cond_wait(&cond, &mutex);
	}
	pthread_mutex_unlock(&mutex);
	printf("Valeur finale : %d\n",valeur);
	pthread_exit((void*)0); return NULL;
}

int main(int argc, char *argv[]) {
	
	int i = 0;			
	int *pi;
	int *valeur;
	
	if(argc != 2)
		return EXIT_FAILURE;
		
	val_thread_max = atoi(argv[1]);
	tab = malloc((atoi(argv[1])+1)*sizeof(pthread_t));
	
	//Création du premier Thread
	pthread_create(&tab[0],NULL,funcAttente,(void *)0);
	
	for (i =1;i<atoi(argv[1])+1;i++) {
		//passage de l'argument par le pointeur
		pi = malloc(sizeof(int));
		*pi = i;	//Pointeur vers variable i;
		pthread_create(&tab[i],NULL,funcThread,pi);
	}
	for (i =1;i<atoi(argv[1])+1;i++) {
		if(pthread_join(tab[i],(void**)&valeur) !=0){
			printf("pthread_join\n");
			exit(1);	
		}
	}	
}