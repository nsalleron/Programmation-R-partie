#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#define PRODUCTEUR int c;  while((c = getchar()) != EOF){ push(c); }
#define CONSOMMATEUR while(1) { putchar(pop()); fflush(stdout); }


#define STACK_SIZE 2


sem_t *mutexpush, *mutexpop, *mutexCondPush, *mutexCondPop;
char *tabStack;

struct pile{
	char ptr[STACK_SIZE];
	int stack_size;
};

struct pile *stack;


void push(int car) /* empiler 1 caractère */
{
		printf("%d | Lancement du push \n",getpid());
		/* Vérrouillage des opérations */
		sem_wait(mutexpush);
		/* Si le tableau est plein */
		while (stack->stack_size == STACK_SIZE){
				printf("%d | -> Attente consommateur\n", getpid());
				 /* Le consommateur doit prendre des caractères */
				 /* On attend qu'il nous réveil pour produire des caractères */
				sem_wait(mutexCondPush);
		}

		printf("%d | Autorisé à empiler\n",getpid());

		/*On est ici si nous sommes autorisé à produire */
		printf("%d | Valeur de stack : %d \n",getpid(),stack->stack_size);
		stack->stack_size++;  /* Note : Dans le premier passage, il passe de -1 à 0 */
		stack->ptr[stack->stack_size] = car ;
		if((int)car == 10)
			printf("%d | Empilation du caractère de fin\n",getpid());
		else
			printf("%d | Empilation de : %c\n", getpid(),stack->ptr[stack->stack_size]);


		/* Si c'est le premier caractère dans le tableau */
		if (stack->stack_size == 0){
			/* Nous pouvons réveiller le consommateur pour qu'il consomme */
			printf("%d | Réveil du consommateur \n", getpid());
			sem_post(mutexCondPop);
		}

		/* Fin des opérations de la thread */
		sem_post(mutexpush);
}

int pop(){ /* dépiler 1 caractère */
		int car;
		printf("%d | Lancement de pop\n",getpid());
		 /* Vérrouillage des opérations */
		sem_wait(mutexpop);
		/* Si le tableau est vide */
		while (stack->stack_size == -1){

				 /* Le producteur doit produire des caractères */
				 /* On attend qu'il nous réveil pour consommer des caractères */
				printf("%d | Le consommateur s'endort\n", getpid());
				//printf("%d | Valeur de stack : %d \n",getpid(), stack->stack_size);
				sem_wait(mutexCondPop);
				printf("%d | Reprise - Valeur de stack : %d\n",getpid(),stack->stack_size);
				
		}
		printf("%d | Autorisé à dépiler\n",getpid());

		/* On est ici si nous sommes autorisé à consommer */
		car =stack->ptr[stack->stack_size];
		stack->stack_size --;
		if((int)car == 10)
			printf("%d | Dépilation du caractère de fin\n",getpid());
		else
			printf("%d | Dépilation de : %c\n",getpid(),car);

		/* Nous pouvons réveiller le producteur pour qu'il produit */
		/* Il reste un emplacement dans le tableau */
		if (stack->stack_size == (STACK_SIZE-1)){
			/* Nous pouvons réveiller le producteur pour qu'il produit */
			printf("%ld | Réveil du producteur\n",getpid());
			sem_post(mutexCondPush);
		}

		 /* Fin des opérations de la thread */
		sem_post(mutexpop);


		return car;
}

void *producteur()
{
	PRODUCTEUR
}


void *consommateur()
{
	CONSOMMATEUR

}


int main (int argc, char* argv[])
{

		int i = 0;
		int shmDesc = 0, shmDescStack = 0;

		if(argc<3)
			return EXIT_FAILURE;

		int nbProducteur = atoi(argv[1]);
		int nbConsommateur = atoi(argv[2]);

		printf("Nombre total : %d\n", (nbConsommateur + nbProducteur));

		//Vérification de la présence du descripteur;
		shm_unlink("monshm");
		shm_unlink("monshmStack");
		sem_unlink("mutexpushsem");
		sem_unlink("mutexpopsem");
		sem_unlink("mutexcondsempop");
		sem_unlink("mutexcondsempush");


		/* 	POUR LA PILE PARTAGEE	+ COMPTEUR	*/

		/* Ouverture de la pile et création dans la mémoire */
		if ((shmDesc = shm_open("monshm",O_RDWR | O_CREAT,0600)) == -1) {
			perror("Echec de l'allocation du descripteur\n");
			exit(errno);
		}

		/* Allocation pour stocker STACK_SIZE entiers*/
		if(ftruncate(shmDesc,sizeof(char)*STACK_SIZE) == -1){
			fprintf(stderr, "Echec d'allocation ftruncate\n");
		}


		/* Segmentation pour stocker STACK_SIZE entiers*/
		if ((stack = mmap(NULL, sizeof(char) * STACK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, shmDesc, 0)) == MAP_FAILED){
			fprintf(stderr, "Echec de l'allocation de la ressource partagee\n");
			//exit(errno);
		}

		
		/* Initialisation */
		stack->stack_size = -1;

		/* Ouverture des mutex */
		
		if((mutexpush = sem_open("mutexpushsem",O_CREAT|O_EXCL|O_RDWR,0666,1)) == SEM_FAILED){
			if(errno != EEXIST){
				perror("sem_open");
				exit(1);
			}
		}

		if((mutexpop = sem_open("mutexpopsem",O_CREAT|O_EXCL|O_RDWR,0666,1)) == SEM_FAILED){
			if(errno != EEXIST){
				perror("sem_open");
				exit(1);
			}
		}

		if((mutexCondPop = sem_open("mutexcondsempop",O_CREAT|O_EXCL|O_RDWR,0666,0)) == SEM_FAILED){
			if(errno != EEXIST){
				perror("sem_open");
				exit(1);
			}
		}
		if((mutexCondPush = sem_open("mutexcondsempush",O_CREAT|O_EXCL|O_RDWR,0666,0)) == SEM_FAILED){
			if(errno != EEXIST){
				perror("sem_open");
				exit(1);
			}
		}

		int *pids = malloc((nbConsommateur + nbProducteur) * sizeof(int));

		for(i = 0; i<nbConsommateur;i++){
			printf("PID : %d | nbConsommateur : %d\n",getpid(),i);
			if(fork()==0)//Consommateur
			{	printf("PID : %d | lancementConsommateur : %d\n",getpid(),i);
				consommateur();
			}

		}
		for(i = nbConsommateur; i<(nbProducteur+nbConsommateur);i++){
			printf("PID : %d | nbProducteur : %d\n",getpid(),i);
			if(fork()==0)//Producteur
			{	printf("PID : %d | lancementProducteur :%d\n",getpid(),i-nbConsommateur);
				producteur();
			}
		}

		wait(NULL);	//Attention il y aura des zombies...
}

