#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

//On reprend l’exercice précédent en ajoutant la contrainte que le processus fils1 ne peut se terminer qu’après que fils2 a affiché son message. Comme toujours, chaque processus doit attendre que leur fils se termine. Pour synchroniser les affichages, on impose comme contrainte de n’utiliser que les signaux SIGUSR1 et SIGUSR2. Vous ne pouvez pas utiliser des sémaphores, ni les fonctions wait, waitpid, sleep etc, ni des fichiers ou des attentes actives.
//
//Vous donnerez le texte d’un tel programme en modifiant celui de l’exercice précédent, et vous direz en commentaire au début du fichier demandé pourquoi il n’est pas possible de n’utiliser qu’un seul type de signal.

//On ne peut pas car les signaux peuvent se superposer... Comment fils1.1 doit envoyer à fils1 sa mort et que fils2 doit prévenir fils1 de la mort de son fils, faire ceci avec un seul signal est impossible.
void *func(void * arg){
	return 0;
}

int main(int argc,char *argv[]){
	pid_t fils[2];
	int i, j;
	int status;
	struct sigaction action;
	
	
	/* Signaux */
	sigset_t sig;
	sigfillset(&sig);
	sigprocmask(SIG_SETMASK, &sig, NULL); //On bloque tout les sigaux;
	action.sa_mask = sig;
	action.sa_handler = (void*)func;
	action.sa_flags = 0;
	sigaction(SIGUSR1, &action, 0);
	sigaction(SIGUSR2, &action, 0);
	
	/*Boucle pour faire les fils */
	for(i = 0;i<2;i++){
		int valFork;		
		if((fils[i] = fork()) == -1){
			perror("fork");
			exit(-1);
		}

		if(fils[i] == 0){ // Les fils
		
			pid_t pfils;	
			if((pfils = fork()) == -1){
				perror("fork");
				exit(EXIT_FAILURE);
			}
			if(pfils == 0){	//les petits fils
				printf("Fils %d.1 - Mon PID : %d, PPID : %d\n",i+1,getpid(),getppid());
				kill(getppid(), SIGUSR1);
				exit(EXIT_SUCCESS);
			}else{
				sigdelset(&sig, SIGUSR1);
				sigsuspend(&sig);
				
				if(i == 1){	//Fils 2
					printf("Fils 2 \tMon PID : %d, PPID : %d, PID de mon frere : %d\n",getpid(),getppid(), fils[0]);
					kill(fils[0], SIGUSR2);
				}
				if(i == 0){ //Fils 1
					sigdelset(&sig, SIGUSR2);
					sigsuspend(&sig);
					printf("Fils 1 \tMon PID : %d, PPID : %d\n",getpid(),getppid());
					kill(getppid(), SIGUSR2);
				}
				exit(EXIT_SUCCESS);		
			}
		}
	}//for
	sigdelset(&sig, SIGUSR2);
	sigsuspend(&sig);
	
	printf("Mon fils %d est mort\n", fils[0]);	
	printf("Mon fils %d est mort\n", fils[1]);
	
}


