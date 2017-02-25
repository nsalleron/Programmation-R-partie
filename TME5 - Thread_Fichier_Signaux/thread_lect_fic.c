#define _XOPEN_SOURCE 700
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include "libupper.h"


//Ecrire un programme C prenant plusieurs noms de fichiers en argument. 

//Il doit créer autant de Threads que de fichiers, et les lancer en parallèle. 
//La i-ème Thread créée doit appliquer la fonction ci-dessus sur le i-ème fichier de la liste des fichiers, et transmettre au programme principal le résultat de cette fonction. 
//Le programme principal attend la terminaison de chaque Thread et teste son retour. S’il n’est pas nul, il affiche le nom du fichier posant problème sur le flux de sortie. 
//Au final, le programme sort avec comme code de retour le nombre de fichiers qui ont posé problème (donc 0 si tout c’est bien passé).

//Pourquoi un void**?
//Chaque thread possède sa propre pile, dans la pile se trouve toute les variables locales.
//int retour dans le thread est locale, elle sera donc dans la pile. Ce sera donc perdu car on va désallouer la pile.
//Erreur de seg : car on essaye d'accéder à une variable non alloué !
//SOLUTION : MALLOC pour réserver une zone mémoire dans le tas.
//Le retour ne sera plus un entier mais un pointeur d'entier.
//Le thread va donc écrire sa valeur de retour dans le tas.
//Pour la récupérer, il faut dans l'apl a join, pouvoir modifier le pointeur.
//Il faut donc donner l'adresse de la variable status. Grace à ce pointeur, join peut modifier pour le faire pointer sur la variable pointeur du thread;




void *funcThread(void *arg){
	int *t = 0;
	t = malloc(sizeof(int));
	printf("ValArg : %s\n",(char*)arg);
	*t = upper((char*)arg);
	pthread_exit((void*)t);
}


int main(int argc, char *argv[]) {
	
	int i;
	if(argc < 2)
		return EXIT_FAILURE;
		
	pthread_t* tabThread;
	tabThread = malloc((argc-1)*sizeof(pthread_t));
	
	char *pchar = 0;
	for (i = 1;i<argc;i++) {
		pchar = argv[i];
		pthread_create(&tabThread[i-1], NULL, funcThread, (void*)pchar);
		printf("thread %d\n",i);
	}
	
	//Attente de l'ensemble des threads;
	void *valRet = 0;
	int valRetour = 0;
	
	for (i = 1;i<argc;i++) {
			pthread_join(tabThread[i-1], (void**)&valRet);
			if((*(int*)valRet) !=0){
				printf("Le fichier %s pb \n",argv[i]);
				valRetour++;
			}
			free(valRet);
	}
	
	return valRetour;
	
	
}