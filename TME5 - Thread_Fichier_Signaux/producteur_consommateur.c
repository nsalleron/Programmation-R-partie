#define _XOPEN_SOURCE 700
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include "producteur_consommateur.h"
//Nous voulons faire communiquer une thread Producteur et une thread Consommateur en utilisant une pile de taille fixe (un tableau de 100 caractères). Les valeurs empilées sont des caractères. La thread Producteur utilise la fonction Push() pour empiler un caractère au sommet de la pile et la thread Consommateur utilise la fonction Pop() pour désempiler une valeur du sommet de la pile. Une variable globale stack_size contrôle le sommet de la pile.

//Programmez les fonctions Push() et Pop() décrites ci-dessus pour faire communiquer les threads Producteur et Consommateur, le corps de ces deux fonctions reposant respectivement sur les deux séquences de code définies par les deux macros suivantes, fournies dans le fichier .h en annexe :

//Ecrire ensuite le programme main utilisant ces deux fonctions.

//Exemple d'appel :
//echo "123456789" | bin/producteur_consommateur

#define STACK_SIZE 100
int stackSize = 0;
char* TabStack;
pthread_mutex_t stack = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condPop = PTHREAD_COND_INITIALIZER;
pthread_cond_t condPush = PTHREAD_COND_INITIALIZER;

void push(char c){
	pthread_mutex_lock(&stack);
	while (stackSize == (STACK_SIZE-1)) {
		
		printf("\nProducteur : %ld en ATTENTE\n",(long)pthread_self());
		pthread_cond_wait(&condPush, &stack);
	}
	
	stackSize++;
	TabStack[stackSize] = c;
	
	//printf("Producteur : %ld : %c\n",pthread_self(),c);
	if(stackSize == 1)
		pthread_cond_signal(&condPop);
	pthread_mutex_unlock(&stack);
	
}


char pop(){
	char c;
	pthread_mutex_lock(&stack);
	while (stackSize < 0) {
		printf("\nConsommateur : %ld en ATTENTE\n",(long)pthread_self());
		pthread_cond_wait(&condPop, &stack);
	}
	stackSize--;
	c = TabStack[stackSize];
	//printf("Consommateur : %ld : %c\n",pthread_self(),c);
	if(stackSize == 99)
		pthread_cond_signal(&condPush);
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
	
	pthread_t thread[2];
	
	pthread_create(&thread[0], NULL, prod, NULL);
	pthread_create(&thread[1], NULL, cons, NULL);
	
	for(i = 0; i<2;i++)
		pthread_join(thread[i], NULL);
	
	
}