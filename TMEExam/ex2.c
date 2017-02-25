#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//Ecrire un programme qui crée deux processus fils fils1 et fils2. Chaque fils crée un fils, fils1.1 et fils2.1 respectivement. Ces 4 processus ne font qu’imprimer leur PID et PPID. Dans le cas du processus fils2, il imprime aussi le PID de son frère aîné fils1. Les processus n’ayant pas de fils se terminent aussitôt, mais un processus qui a des fils (y compris le processus principal) ne se termine qu’après ceux-ci.
//
//Contrainte : ne pas utiliser les IPC, ni des fichiers, ni la fonction sleep. En revanche on considère que toutes les fonctions utilisées, notamment fork, réussissent toujours.



int main(int argc,char *argv[]){
	
	pid_t fils[2];
	pid_t pfils; 
	int i, j, status;
	printf("Père : %d\n",getpid());
	
	for(i = 0;i<2;i++){
		
		int valFork;    
		            
		if((fils[i] = fork()) == -1){
			perror("fork");
			exit(EXIT_FAILURE);
		}
		
		if(fils[i] == 0){ // Les fils
		
			if(i == 0)        //fils 1
				printf("Fils 1 \tMon PID : %d, PPID : %d\n",getpid(),getppid());
			else       //fils 2
				printf("Fils 2 \tMon PID : %d, PPID : %d, PID de mon frere : %d\n",getpid(),getppid(), fils[0]);

			if((pfils = fork()) == -1){
				perror("fork");
				exit(EXIT_FAILURE);
			}
			
			if(pfils == 0){        //les petits fils
				printf("Fils %d.1 - Mon PID : %d, PPID : %d\n",i+1,getpid(),getppid());
				exit(EXIT_SUCCESS);
			}else{
				wait(&status);        
				printf("Mon petit fils est mort\n");        
				exit(EXIT_SUCCESS);               
			}
		}
	}//for

	for(j = 0;j<2;j++){
		//printf("Attente de : %d\n",fils[j]);
		waitpid(fils[j],&status,0);
		printf("Mon fils %d est mort\n", fils[j]);        
	}
}//main