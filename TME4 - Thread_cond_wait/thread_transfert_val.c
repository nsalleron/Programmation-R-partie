#define _POSIX_SOURCE 1
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

//Ecrire un programme créeant N processus légers (à l’aide de pthread_create) et passant en paramètre à chacune son numéro de création compris entre 0 et N. Chacune affichera son numéro de création et son identité (utiliser pthread_self). Ensuite elle se termine, avec pthread_exit, en retournant son numéro de création multiplié par 2. De son côté, le programme principal doit attendre leur terminaison (à l’aide de pthread_join) en affichant la valeur renvoyée par chaque.

//Exemple d'appel :
//$PWD/bin/thread_create 10

void *funcThread(void *arg){
	
	int *pt; //Pointeur
	pt =  malloc(sizeof(int)); //On alloue la taille d'un entier dedans, il est vide pour le moment
	printf("Numéro de création %d et identité %ld\n",(*(int*)arg),(long)pthread_self());
	*pt = (*(int*)arg)<<1; //On multiplie la valeur par deux avec conversion du void* en int* + accès valeur 
	free(arg); //libération
	pthread_exit((void*)pt); return NULL; //On cast le pointeur vers un pointeur de void
}

pthread_t *tab;
int main(int argc, char *argv[]) {
	
	int i = 0;			
	int *pi;
 	void *valeur = 0; //Pointeur pour la valeur de retour
	
	if(argc != 2)
		return EXIT_FAILURE;
		
	tab = malloc(atoi(argv[1])*sizeof(pthread_t));
		
	for (i =0;i<atoi(argv[1]);i++) {
		//passage de l'argument par le pointeur
		pi = malloc(sizeof(int));
		*pi = i;	//Pointeur vers variable i;
		pthread_create(&tab[i],NULL,(void*)funcThread,pi);
	}
	
	for (i =0;i<atoi(argv[1]);i++) {
		//valeur = malloc(sizeof(int));	
		if(pthread_join(tab[i],(void**)&valeur) !=0){
			printf("pthread_join\n");
			exit(1);	
		}
		printf("La valeur de %ld est %d\n",(long)tab[i], *(int*)valeur); //Cast du void* vers int* + accès valeur;
	}

	
	
}