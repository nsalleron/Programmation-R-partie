#define _POSIX_SOURCE 1
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>


//En utilisant la fonction pthread_detach, modifier le programme précédent pour que les threads affichant la valeur aléatoire soient détachées après leur création.

pthread_t *tab;
int val = 0;
int valTab = 0;
int condition = 0;
static int valThread = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *funcThread(int *arg){
	
	//Detachement
	pthread_detach(pthread_self());
	
	valThread++;
	pthread_mutex_lock(&mutex);
	val += *arg;
	printf("Valeur de val : %d\n",val);
	//printf("Valeur de valTab%d\n",valTab);
	
	if(valThread == valTab){	//On est dans le dernier Thread;
		condition = 1;
		pthread_cond_signal(&cond);
	}
	pthread_mutex_unlock(&mutex);
	
	pthread_exit((void*)0); return NULL;
}

void *funcAttente(void *arg){
	
	//Detachement
	pthread_detach(pthread_self());
	
	pthread_mutex_lock(&mutex);
	//ici il n'y a qu'un seul thread qui doit attendre.
	while(!condition)
		pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);
	printf("Valeur finale : %d\n",val);
	pthread_exit((void*)0);
	
}

int main(int argc, char *argv[]) {
	
	int i = 0;			
	int *pi;
	int *valeur;
	
	if(argc != 2)
		return EXIT_FAILURE;
		
	valTab = atoi(argv[1]);
	tab = malloc((atoi(argv[1])+1)*sizeof(pthread_t));
	
	//Création du premier Thread
	pthread_create(&tab[0],NULL,funcAttente,NULL);
	
	
	for (i =1;i<atoi(argv[1])+1;i++) {
		//passage de l'argument par le pointeur
		pi = malloc(sizeof(int));
		*pi = (int) (10*((double)rand())/ RAND_MAX)	;
		pthread_create(&tab[i],NULL,funcThread,pi);
	}
	
	for (i =0;i<valTab+1;i++) {
		//passage de l'argument par le pointeur
		pthread_join(tab[i], NULL);
	}	
	
}