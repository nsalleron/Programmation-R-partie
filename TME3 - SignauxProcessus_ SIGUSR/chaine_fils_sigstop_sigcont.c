#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>


//On reprend encore le même exercice, mais nous voulons que tous les processus, à l’exception du processus initial, soient suspendus par un signal SIGSTOP. 
//Lorsqu’ils le sont tous, le processus initial affiche : Tous les descendants sont suspendus. L’exécution de ces processus doit alors reprendre pour que ceux-ci se terminent. Lorsque tous se sont terminés, le programme initial affiche Fin du programme. De nouveau, il faut répondre sans utiliser les fonctions de la famille wait.


pid_t *Proc;
int i = 0;
pid_t pere;
int k = 1;
int status = 0;

void sigTrt(int sig){
	printf("%d | passage \n",getpid());
	if(sig == SIGCHLD){
		if(i==0){
			if(status == 0){
				printf("Tous les descendants sont suspendus.\n");
				kill(Proc[0], SIGCONT);
			}
			if(status == 1){
				printf("Fin programme.\n");
				
			}
			status = 1;
		}else {
			if(status == 0){
				printf("%d | Je m'arrête.\n",getpid());
				status = 1;
				kill(getpid(), SIGSTOP);
			}
			if(status == 1){
				printf("%d | SIGCONT à i+1.\n",getpid());
				kill(Proc[i+1], SIGCONT);
			}
			
		}
	}
	
}

int main(int argc, char *argv[]) {
	
	if(argc != 2){
		perror("exiting...");
		return 0;
	}
	
	int n = atoi(argv[1]) + 1;
	int j = 0;
	
	Proc = malloc(n*sizeof(pid_t));
	
	sigset_t sig;
	sigemptyset(&sig);
	
	
	struct sigaction action;
	action.sa_flags = 0;
	action.sa_mask = sig;
	action.sa_handler = sigTrt;
	sigaction(SIGCHLD, &action, 0);
	
	for(i = 0;i<n;i++){
		if((Proc[i] = fork()) != 0){//Père
			break;
		}else{
			Proc[i] = getppid();
			if((i+1)==n){	//On est dans le dernier
				Proc[i+1] = getpid();
				for(j=0;j<n;j++)
					printf("PID du processus %d : %d\n",j,Proc[j]);
				printf("Je m'arrête 1er.\n");
				kill(getpid(),SIGSTOP);	//Traitement signal synchrone quand c'est à sois même. 
				printf("Reprise\n");
				exit(EXIT_SUCCESS);
			}
		}
	}
	sigsuspend(&sig);
	if(i==0)
		sigsuspend(&sig);
	
	
}
