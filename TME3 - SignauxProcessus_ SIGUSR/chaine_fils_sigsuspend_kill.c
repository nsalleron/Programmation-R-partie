#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

//On reprend l’exercice précédent, mais on s’interdit d’utiliser les fonctions wait et assimilées, ni bien sûr le signal SIGCHLD. A la place on utilisera les fonctions kill, sigaction et sigsuspend. Comment résoudre alors le problème en s’assurant qu’aucun processus ne se termine avant que tous les autres ne soient créées ?
//
//Remarque : on ne demande plus de récupérer la valeur aléatoire ici.
//
//Exemple d'appel :
//$PWD/bin/kill_proc 10

pid_t *Proc;

void sigTrt(int sig){
	if(sig == SIGUSR1)
		printf("PID : %d : Fin d'un fils.\n",(int)getpid());
}

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
	Proc[0] = getpid(); //PID du premier papa
	
	/* Traitement du signal */
	
	sigset_t sig;
	sigfillset(&sig); //On remplit l'ensemble pour masquer tout les signaux
	sigprocmask(SIG_SETMASK, &sig, NULL); //On bloque les sigaux;
	struct sigaction action;
	action.sa_flags = 0;
	action.sa_mask = sig;
	action.sa_handler = sigTrt;
	sigaction(SIGUSR1, &action, NULL);	//On change l'action par défaut de SIGUSR1 pour ne pas avoir le comportement par défaut.
	
	
	/* Création des processus */
	
	for(i = 1;i < n;i++){
		if((Proc[i] = fork()) != 0){//Père
			break;
		}else{
			Proc[i] = getpid();
			//printf("%d : PID : %d\n",i, getpid());
			if((i+1)==n){	//On est dans le dernier
				for(j=0;j<n;j++)
					printf("%d | PID du processus %d : %d\n",getpid(),j,Proc[j]);
				kill(getppid(),SIGUSR1);
				break;
			}
		}
		
		
	}
	
	sigdelset(&sig, SIGUSR1);//sigusr1 non bloqué
	//Attente du fils nouvellement crée
	if(i != n){
		
		sigsuspend(&sig);	//L'ancien masque est réinstallé au retour.
		printf("PID : %d\n",getpid());
		
		if(getpid()!=Proc[0]){
			kill(getppid(),SIGUSR1);
		}else{
			printf("j'étais le premier père\n");
		}
	}
}



