#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>


//1 Chaîne de processus
//
//A l’aide de la fonction fork, ecrire un programme qui crée une chaîne de processus telle que le processus initial (celui du main) crée un processus qui à son tour en crée un second et ainsi de suite jusqu’à la création de N processus (en plus du processus initial). Au moment de sa création, le dernier processus de la chaîne affiche le Pid de tous les autres processus y compris celui du processus initial. Chacun des autres processus attend la terminaison de son fils, puis affiche son propre Pid (à l’aide de getpid), celui de son père (à l’aide de getppid) et celui de son fils avant de se terminer.
//
//On souhaite de plus que le dernier processus créé génère une valeur aléatoire entre 0 et 100. Pour générer cette valeur aléatoire utilisez :
//
//(int)(rand () /(((double) RAND_MAX +1) /100))
//
//Ecrire le programme de sorte que le processus initial affiche cette valeur aléatoire avant de se terminer
//
//Exemple d'appel :
//$PWD/bin/chaine_proc 10
pid_t *Proc;

int main(int argc, char *argv[]) {
	
	if(argc != 2){
		perror("exiting...");
		return 0;
	}
	
	int n = atoi(argv[1]) + 1;
	
	int i = 0;
	int j = 0;
	int status;
	
	Proc = malloc(n*sizeof(pid_t));
	
	Proc[0] = getpid(); //PID du père originel;
	
	for(i = 1;i < n;i++){
		
		if((Proc[i] = fork()) != 0){//Père
			printf("PID à l'emplacement %d : %d\n",i,Proc[i]);
			//printf("Mon PID : %d\n",getpid());
			break;
			
		}else{
			
			srand(getpid()); //A savoir
			
			Proc[i] = getpid();
			
			//printf("%d",i);
			
			if((i+1)==n){	//On est dans le dernier
			
				for(j = 0;j < n;j++)
					printf("PID du processus %d : %d\n",j,Proc[j]);
					
				status = (int)(rand () /(((double) RAND_MAX +1) /100));
				
				//status = 15;
				
				printf("%d | Valeur aléatoire dernier fils : %d\n",getpid(),status);
				
				exit(status);
			}
		}
		
		
	}
	//Attente du fils nouvellement crée
	waitpid(Proc[i], &status, 0);
	if(i==1){
		printf("%d | Valeur aléatoire : %d\n",getpid(),WEXITSTATUS(status));
		return EXIT_SUCCESS;
	}
		
	exit(WEXITSTATUS(status));
		
	
}
