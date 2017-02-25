#define _POSIX_SOURCE 1
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

//Une barrière est un mécanisme de synchronisation. Elle permet à N threads de prendre rendez-vous en un point donné de leur exécution. 
//Dès que l’une d’entre elles atteint la barrière, elle reste bloquée jusqu’à ce que toutes les autres y arrivent. Lorsque toutes sont arrivées, chacune peut alors reprendre son exécution.

//Ecrire une fonction, qu’on nommera wait_barrier prenant en argument un entier N, permettant à N threads de se synchroniser sur une barrière. Testez votre programme avec la thread suivante :
//
//void* thread_func (void *arg) {
//	printf ("avant barriere\n");
//	wait_barrier (((int *)args)[0]);
//	printf ("après barriere\n");
//	pthread_exit ( NULL);
//}
//En exécutant votre programme avec 2 threads, il devra afficher :
//
//avant barrière
//avant barrière
//après barrière
//après barrière
//En d’autres termes, on veut que tous les messages « avant barrière » soient affichés avant les messages « après barrière ».
//
//Exemple d'appel :
//$PWD/bin/thread_broadcast 10

int val_thread_max;
int val_inside_barrier=0;
int condition = 0;
int *args;
pthread_t *tabThread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void wait_barrier(int arg){
	
	
	pthread_mutex_lock(&mutex);
	val_inside_barrier++;
	if (val_inside_barrier == arg) {
		condition = 1;
		pthread_cond_broadcast(&cond);
	}else {
		while(!condition){
			pthread_cond_wait(&cond, &mutex);
		}
	}
	pthread_mutex_unlock(&mutex);
	return;
}

void* thread_func (void *arg) {
	printf ("%ld | avant barriere\n",(long)pthread_self());
	wait_barrier(((int *)args)[0]);
	printf ("%ld | après barriere\n",(long)pthread_self());
	pthread_exit ( NULL);
}




int main(int argc, char *argv[]) {
	
	int i = 0;			
	int *pi;
	int *valeur;
	
	if(argc != 2)
		return EXIT_FAILURE;
		
	val_thread_max = atoi(argv[1]);
	
	tabThread = malloc(val_thread_max *sizeof(pthread_t));
	args = malloc(val_thread_max *sizeof(int));


	for (i =0;i<val_thread_max;i++) {
		//passage de l'argument par le pointeur
		args[i] = val_thread_max - i;
		printf("args[%d] = %d\n", i,args[i]);
		pi = malloc(sizeof(int));
		*pi = i;
		pthread_create(&tabThread[i],NULL,thread_func,pi);
	}
	
	for (i = 0; i < val_thread_max; i++) {
		if(pthread_join(tabThread[i],NULL ) !=0){
			printf("pthread_join\n");
			exit(1);	
		}
		
	}
}