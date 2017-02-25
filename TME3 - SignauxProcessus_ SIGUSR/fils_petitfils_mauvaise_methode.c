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

//On reprend l’énoncé de l’exercice "Attente de processus" où un processus crée deux fils créant chacun un processus. 
//On ajoute la contrainte que le processus fils 1 ne peut se terminer qu’après les affichages réalisés par son frère fils 2 et son fils fils 1.1. 
//Comme auparavant, les processus ne se terminent qu’après envoi des messages de leur fils respectifs. Trouvez une solution n’utilisant que les signaux SIGUSR1 et SIGUSR2, à l’exclusion de tout autre moyen (Wait, Sleep etc).

int main(int argc,char *argv[]){
	pid_t fils[2];
	int i, j;
	int status;
	printf("Père : %d\n",getpid());
	
	sigset_t sig;
	sigfillset(&sig);
	sigprocmask(SIG_SETMASK, &sig, NULL); //On bloque tout les sigaux;
	
	
	
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
				if(i == 1){	//Fils 2
					while(1){
						//printf("Pas encore...\n");
						sigpending(&sig);
						if(sigismember(&sig, SIGUSR1))	//Il doit attendre la fin de son fils
							break;
					}
					printf("Fils 2 \tMon PID : %d, PPID : %d, PID de mon frere : %d\n",getpid(),getppid(), fils[0]);
					kill(fils[0], SIGUSR2);
					kill(getppid(), SIGUSR2);
				}
				if(i == 0){ //Fils 1
					while(1){
						//printf("Pas encore...\n");
						sigpending(&sig);
						if(sigismember(&sig, SIGUSR1) && sigismember(&sig, SIGUSR2))
							break;
					}
					printf("Fils 1 \tMon PID : %d, PPID : %d\n",getpid(),getppid());
					kill(getppid(), SIGUSR1);
				}
				exit(EXIT_SUCCESS);		
			}
		}
	}//for
	
	while(1){
		//printf("Pas encore...\n");
		sigpending(&sig);
		if(sigismember(&sig, SIGUSR1) && sigismember(&sig, SIGUSR2))
			break;
	}
		printf("Mon fils %d est mort\n", fils[0]);	
		printf("Mon fils %d est mort\n", fils[1]);
	
}//main





