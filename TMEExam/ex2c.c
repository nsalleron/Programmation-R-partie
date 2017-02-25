#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/wait.h>

//Ecrire un programme qui crée deux processus fils fils1 et fils2. Chaque fils crée un fils, fils1.1 et fils2.1 respectivement. Ces 4 processus ne font qu’imprimer leur PID et PPID. Dans le cas du processus fils2, il imprime aussi le PID de son frère aîné fils1. Les processus n’ayant pas de fils se terminent aussitôt, mais un processus qui a des fils (y compris le processus principal) ne se termine qu’après ceux-ci.
//
//Contrainte : ne pas utiliser les IPC, ni des fichiers, ni la fonction sleep. En revanche on considère que toutes les fonctions utilisées, notamment fork, réussissent toujours.

int main (int argc, char ** argv){ 
	
	int i=1; 
	int j=0; 
	int ret, nb_fils;
	pid_t fils1, principal;

	if (argc > 1) {
		fprintf(stderr, "Usage: %s\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	principal = getpid ();

	/*création des processus */
	while (i<= 2)
		if ((ret=fork ())) { //Le père passe là dedans 
		
			if (i==1)
				fils1 = ret; //Premier fils du père  
				
			 i++;
		}else {
			ret = fork ();	//Création des petits fils. 
			break ;
		}
	
	/* affichage */
	if (getpid ()!= principal){
		printf ("pid %d, pid pere: %d \n", getpid (),getppid ()); 
	if ((ret !=0) && (i==2) )
			/* fils 2*/
			printf ("fils1 :%d\n", fils1);  
	} 


	if (ret!=0) { //Le père attend les fils qui attendent les petits-fils. 
		/* processus a des fils */
		nb_fils=1;
		if (getpid ()== principal)
			 nb_fils++;
		for (j=0;j<nb_fils;j++)   
			wait (NULL);
	}
}