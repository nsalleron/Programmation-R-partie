#define _XOPEN_SOURCE 700
#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>

#define SIZE 2


int pile[SIZE];  //Tableau de 100 éléments
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
    while (stack_size == SIZE){
      
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
      pthread_cond_signal(&cond_push);  // On envoie le signal sur ce que lui attend, push cad nous.
    }
    
    /* Fin des opérations de la thread */
    pthread_mutex_unlock(&mutex_stack);
}

int  pop(){ /* dépiler 1 caractère */
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
    if (stack_size == (SIZE-1)){
      /* Nous pouvons réveiller le producteur pour qu'il produit */
      printf("%ld | Réveil du producteur\n",(long ) pthread_self());
      pthread_cond_signal(&cond_pop);
    }
    
     /* Fin des opérations de la thread */
    pthread_mutex_unlock(&mutex_stack);
    
    return car;
}

void *producteur()
{

    int c;  
    while((c = getchar()) != EOF)
    { 
        push(c); 
    }
    
    pthread_exit ((void*)0);    
}


void *consommateur()
{
/* Version original */
//    while(1) 
//    {
//      putchar(pop());
//    	fflush(stdout); 
//    }
    while(1) 
    {
      //putchar(pop());
      pop(); //<- pour démonstration (l'affichage est inclut dans la fonction pop
    	fflush(stdout); 
    }
    
    
    pthread_exit ((void*)0);
}



int main (int argc, char* argv[])
{

    int *status;
    pthread_t pth1, pth2;
    stack_size = -1;
    
    
    /* producteur */
    if (pthread_create (&pth1, NULL, producteur, (void*)0) != 0) {
        perror("pthread_create \n");
        exit (1);
    }

    /* consommateur */
    if (pthread_create (&pth2, NULL, consommateur, (void*)0) != 0) {
        perror("pthread_create \n");
        exit (1);
    }  
    
                       
    /* consommateur */
    if (pthread_join(pth1, (void**) &status) != 0) {
        printf ("pthread_join"); 
        exit (1);
    }

    /* producteur */  
    if (pthread_join(pth2, (void**) &status) != 0) {
        printf ("pthread_join"); 
        exit (1);
    }

    
    return 0;

  }
