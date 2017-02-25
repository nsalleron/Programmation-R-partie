#define _XOPEN_SOURCE 700
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "producteur_consommateur.h"

//Reprenez l’exercice précédent en faisant échanger plusieurs producteurs et plusieurs consommateurs de façon concurrente, toujours à travers une seule pile. Votre programme prendra en argument deux nombres : le nombre de producteurs suivi du nombre de consommateurs.

#define STACK_SIZE 100
int stackSize = 0;
char* TabStack;
pthread_mutex_t stack = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condPop = PTHREAD_COND_INITIALIZER;
pthread_cond_t condPush = PTHREAD_COND_INITIALIZER;

void push(char c){
	pthread_mutex_lock(&stack);
	while (stackSize == 99) {
		printf("\nProducteur : %ld en ATTENTE\n",(long)pthread_self());
		pthread_cond_wait(&condPush, &stack);
	}
	stackSize++;
	TabStack[stackSize] = c;
	//printf("Producteur : %ld : %c\n",(long)pthread_self(),c);
	if(stackSize == 1)
		pthread_cond_broadcast(&condPop);
	pthread_mutex_unlock(&stack);
	
}


char pop(){
	char c;
	pthread_mutex_lock(&stack);
	while (stackSize == 0) {
		printf("\nConsommateur : %ld en ATTENTE\n",(long)pthread_self());
		pthread_cond_wait(&condPop, &stack);
		printf("\nConsommateur : %ld reprise\n",(long)pthread_self());

	}
	stackSize--;
	c = TabStack[stackSize];
	//printf("Consommateur : %ld : %c\n",(long)pthread_self(),c);
	if(stackSize == 100)
		pthread_cond_broadcast(&condPush);
	pthread_mutex_unlock(&stack);
	return c;
	
}


void *prod(){
	PRODUCTEUR
	pthread_exit((void*)0);
}
void *cons(){
	CONSOMMATEUR
	pthread_exit((void*)0);
}

int main(int argc, char *argv[]) {
	int i = 0;
	TabStack = malloc(STACK_SIZE*sizeof(char));
	
	if(argc<3)
		return EXIT_FAILURE;
	
	
	int nbProducteur = atoi(argv[1]);
	int nbConsommateur = atoi(argv[2]);
	
	printf("Nombre total : %d\n", (nbConsommateur + nbProducteur));
	//printf("Nombre total de l'entrée : %ld",strlen(STDIN_FILENO));
	
	
	pthread_t *thread = malloc((nbConsommateur + nbProducteur) * sizeof(pthread_t));
	
	
	
	for(i = 0; i<nbConsommateur;i++)
		pthread_create(&thread[i], NULL, cons, NULL);
		
	for(i = 0; i<nbProducteur;i++)
		pthread_create(&thread[i+nbConsommateur], NULL, prod, NULL);

	for(i = 0; i<(nbConsommateur+nbProducteur);i++)
		printf("Valeur du tableau : %ld\n", (long) thread[i]);
	
	for(i = 0; i<(nbConsommateur+nbProducteur);i++)
		pthread_join(thread[i], NULL);
}