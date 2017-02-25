#define _POSIX_SOURCE 1
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

//Modifier encore une fois pour que les threads soient dans l’état détaché lors de leur création, grâce à la fonction pthread_attr_setdetachstate.
//
//Exemple d'appel :
//$PWD/bin/thread_setdetach 10
//Fichier à créer : src/thread_setdetach.c



pthread_t *tab;
int val = 0;
int valTab = 0;
int condition = 0;
static int valThread = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_attr_t attr;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *funcThread(void *arg){
	pthread_mutex_lock(&mutex);
	valThread++;
	val += (*(int*) arg);
	printf("Valeur de val : %d\n",val);
	if(valThread == valTab){	//On est dans le dernier Thread;
		condition = 1;
		pthread_cond_signal(&cond);
	}
	pthread_mutex_unlock(&mutex);
	pthread_exit((void*)0); return NULL;
}

void *funcAttente(void *arg){
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
	tab = malloc((valTab+1)*sizeof(pthread_t));
	
	//Création du premier Thread
	pthread_create(&tab[0],NULL,funcAttente,NULL);
	
	//Detachment
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);	
	
	for (i =1;i<valTab+1;i++) {
		//passage de l'argument par le pointeur
		pi = malloc(sizeof(int));
		*pi = (int) (10*((double)rand())/ RAND_MAX)	;
		pthread_create(&tab[i],&attr,funcThread,pi);
	}
	
	
	pthread_join(tab[0], NULL);
	

}