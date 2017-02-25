#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>


//On souhaite écrire un variante de la fonction précédente, de sorte que toutes les Threads d’un niveau L soient créées avant de commencer à créer celles du niveau L+1. Pour cela, écrire une fonction main qui
//
//prend sur sa ligne de commande la profondeur de Thread désirée ;
//lance une première Thread qui se bloque en attendant que toutes celles du niveau courant soient créées (utiliser la formule demandée à la question précédente) ;
//lance la variante de la fonction thread_func désirée ;
//affiche pour finir le nombre total de Threads créées (à partir d’une variable incrémentée à chaque création).
//Exemple d'appel :

int cond =0;
pthread_cond_t condthread = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int max = 0;

void* thread_func(void* arg) {
		
		int i, nb;
		int *param;
		int *lvl = (int*)arg;
		pthread_t *tid;
		
		nb = (*lvl)+1;
		
		pthread_mutex_lock(&mutex);
		while (!cond) {
			pthread_cond_wait(&condthread, &mutex);
		}
		cond = 0;
		pthread_mutex_unlock(&mutex);
		
		if (*lvl < max) {
			param = (int*)malloc(sizeof(int));
			*param = nb;
			tid = calloc(nb, sizeof(pthread_t));
			printf("%d cree %d fils\n", (int)pthread_self(), nb);
			pthread_mutex_lock(&mutex);
			for (i = 0; i < nb; i++) {
				pthread_create((tid+i), 0, thread_func, param);
			}
			pthread_cond_broadcast(&condthread);
			pthread_mutex_unlock(&mutex);
			cond = 1;
			for (i = 0; i < nb; i++)
				pthread_join(tid[i], NULL);
		}

		if (*lvl > 1)
			pthread_exit ( (void*)0);
		
		return (void*)0;
	}

int main(int argc, char *argv[]) {
	
	if(argc<2)
		return EXIT_FAILURE;
		
	max = atoi(argv[1]);
	printf("Passage\n");
	
	int *pi = 0;
	int i = 1;
	pi = malloc(sizeof(int));
	*pi = i;
	
	pthread_t pthread;
	printf("Passage\n");
	
	pthread_mutex_lock(&mutex);
	pthread_create(&pthread, NULL, thread_func,(void*)pi );
	pthread_cond_broadcast(&condthread);
	cond = 1;
	pthread_mutex_unlock(&mutex);
	
	pthread_join(pthread, NULL);
}