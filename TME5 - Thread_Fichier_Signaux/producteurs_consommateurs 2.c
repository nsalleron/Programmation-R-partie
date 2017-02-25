#define _XOPEN_SOURCE 700
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#define PRODUCTEUR int c;  while((c = getchar()) != EOF){ push(c); }
#define CONSOMMATEUR while(1) { putchar(pop()); fflush(stdout); }


#define STACK_SIZE 2


int pile[STACK_SIZE];  //Tableau de 100 éléments
int stack_size = -1;  //Pour le moment il n'y a aucun élément dans mon tableau, il est vide.

/* Les conditions */
pthread_mutex_t mutex_stack = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_pop  = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_push  = PTHREAD_COND_INITIALIZER;

void push(int car) /* empiler 1 caractère */

{
		/* Vérrouillage des opérations de la thread */
		pthread_mutex_lock(&mutex_stack);
		
		/* Si le tableau est plein */
		while (stack_size == STACK_SIZE){
			
					printf("%ld | -> Attente consommateur\n", (long ) pthread_self());
				 /* Le consommateur doit prendre des caractères */
				 /* On attend qu'il nous réveil pour produire des caractères */
				 pthread_cond_wait(&cond_pop,&mutex_stack);      
		}
		
		/*On est ici si nous sommes autorisé à produire */
		stack_size++;  /* Note : Dans le premier passage, il passe de -1 à 0 */
		pile[stack_size] = car ;
		if((int)car == 10)
			printf("%ld | Empilation du caractère de fin\n",(long ) pthread_self());
		else
			printf("%ld | Empilation de : %c\n",(long ) pthread_self(), pile[stack_size]);
		/* Incrémentation de la taille du tableau */
		
		
		/* Si c'est le premier caractère dans le tableau */
		if (stack_size == 0){
			/* Nous pouvons réveiller le consommateur pour qu'il consomme */
			printf("%ld | Réveil du consommateur \n", (long ) pthread_self());
			pthread_cond_broadcast(&cond_push);  // On envoie le signal sur ce que lui attend, push cad nous.
		}
		
		/* Fin des opérations de la thread */
		pthread_mutex_unlock(&mutex_stack);
}

int pop(){ /* dépiler 1 caractère */
		int car;
		
		 /* Vérrouillage des opérations de la thread */
		pthread_mutex_lock(&mutex_stack);
		
		/* Si le tableau est vide */
		while (stack_size == -1){
			
				 /* Le producteur doit produire des caractères */
				 /* On attend qu'il nous réveil pour consommer des caractères */
				 printf("%ld | Le consommateur s'endort\n", (long ) pthread_self());
				 pthread_cond_wait(&cond_push,&mutex_stack);
				 //printf("%ld | Valeur de stacksize au retour de condwait : %d \n", (long ) pthread_self(),stack_size);
		}
		
		/* On est ici si nous sommes autorisé à consommer */
		car = pile[stack_size];
		stack_size --;
		if((int)car == 10)
			printf("%ld | Dépilation du caractère de fin\n",(long ) pthread_self());
		else
			printf("%ld | Dépilation de : %c\n",(long ) pthread_self(),car);
		
		/* Nous pouvons réveiller le producteur pour qu'il produit */
		/* Il reste un emplacement dans le tableau */
		if (stack_size == (STACK_SIZE-1)){
			/* Nous pouvons réveiller le producteur pour qu'il produit */
			printf("%ld | Réveil du producteur\n",(long ) pthread_self());
			pthread_cond_broadcast(&cond_pop);
		}
		
		 /* Fin des opérations de la thread */
		pthread_mutex_unlock(&mutex_stack);
		
		
		return car;
}

void *producteur()
{

	PRODUCTEUR
	pthread_exit ((void*)0);    
}


void *consommateur()
{
	CONSOMMATEUR
	pthread_exit ((void*)0);
}

pthread_t *TabStack;

int main (int argc, char* argv[])
{

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
			pthread_create(&thread[i], NULL, consommateur, NULL);
			
		for(i = 0; i<nbProducteur;i++)
			pthread_create(&thread[i+nbConsommateur], NULL, producteur, NULL);

		for(i = 0; i<(nbConsommateur+nbProducteur);i++)
			printf("Valeur du tableau : %ld\n", (long) thread[i]);
		
		for(i = 0; i<(nbConsommateur+nbProducteur);i++)
			pthread_join(thread[i], NULL);

	}
