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

//Nous avons un processus P1 qui crée un processus P2 (fils de P1) qui à son tour crée un processus P3 (fils de P2 et petit-fils de P1). 
//Lorsque le processus P3 est créé, il envoie un signal à son grand-père, le processus P1, pour lui signaler sa création, puis se termine juste après. 
// Quand son père, le processus P2, prend connaissance de la terminaison de P3, il envoie un signal à P1, son père, pour signaler la mort de son fils. 
//Après P2 se termine lui aussi. Le processus P1 doit traiter les événements dans l’ordre décrit ci-dessus. Autrement dit, il doit premièrement traiter la délivrance du signal de P3 en affichant le message « Processus P3 créé », ensuite la délivrance du signal de P2 en affichant « Processus P3 terminé » et à la fin afficher « Processus P2 terminé » lorsqu’il prend connaissance de la mort de son fils.

//Programmez une telle synchronisation.

void sigTrt(int sig){
	if(sig == SIGUSR1)
		printf("%d | Processus P3 crée\n",getpid());
	if(sig == SIGUSR2)
		printf("%d | Processus P3 terminé\n",getpid());
}

int main(int argc, char *argv[]) {
	
	pid_t P1, P2, P3;
	int status;
	struct sigaction action;
	sigset_t ens_sig;
	
	sigfillset(&ens_sig);
	sigdelset(&ens_sig, SIGUSR1);
	sigdelset(&ens_sig, SIGUSR2);
	action.sa_mask = ens_sig;
	action.sa_flags = 0;
	action.sa_handler = sigTrt;
	sigaction(SIGUSR1,&action,NULL);
	sigaction(SIGUSR2,&action,NULL);
	P1 = getpid();
	
	if((P2 = fork()) == -1)
		perror("fork");
	if(P2 == 0){	//P2

		if((P3 = fork()) == -1)
			perror("fork");
			
		if(P3 == 0){	//P3
			//printf("fils P3\n ->kill vers %d\n",P1);
			kill(P1, SIGUSR1); // Le processus se termine
		}else {	//P2
			wait(&status);	//Connaissance mort P3
			kill(P1, SIGUSR2);	//Information vers P1
			//printf("P2 : Mon fils est mort !\n -> kill vers %d",P1);
			
		}
	}else{	//P1
		sigsuspend(&ens_sig);
		sigsuspend(&ens_sig);
		printf("%d | Processus P2 terminé\n",getpid());
		wait(&status);
		//sleep(5);
	}
	
	
}