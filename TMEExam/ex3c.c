#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <signal.h>

//On reprend l’exercice précédent en ajoutant la contrainte que le processus fils1 ne peut se terminer qu’après que fils2 a affiché son message. Comme toujours, chaque processus doit attendre que leur fils se termine. 

//Pour synchroniser les affichages, on impose comme contrainte de n’utiliser que les signaux SIGUSR1 et SIGUSR2. Vous ne pouvez pas utiliser des sémaphores, ni les fonctions wait, waitpid, sleep etc, ni des fichiers ou des attentes actives.
//
//Vous donnerez le texte d’un tel programme en modifiant celui de l’exercice précédent, et vous direz en commentaire au début du fichier demandé pourquoi il n’est pas possible de n’utiliser qu’un seul type de signal.

//On ne peut pas car les signaux peuvent se superposer... Comment fils1.1 doit envoyer à fils1 sa mort et que fils2 doit prévenir fils1 de la mort de son fils, faire ceci avec un seul signal est impossible.

void sig_hand(int sig){
	printf ("signal recu %d %d \n",sig, getpid ());
}

int main (int argc, char ** argv)

{ 
	pid_t fils1, principal;
	int i=1; int ret;
	struct sigaction action;
	sigset_t sig_proc = (sigset_t) NULL;

	if (argc > 1) {
		fprintf(stderr, "Usage: %s\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	action.sa_mask=sig_proc;
	action.sa_flags=0;   
	action.sa_handler = sig_hand;
	sigaction(SIGUSR1, &action,NULL);
	sigaction(SIGUSR2, &action,NULL);

	/* masquer SIGUSR1 et SIGUSR2*/
	sigaddset (&sig_proc, SIGUSR1);
	sigaddset (&sig_proc, SIGUSR2);
	sigprocmask (SIG_SETMASK, &sig_proc, NULL);
		 
	principal = getpid ();

	while (i<= 2)
		if ((ret=fork ())) {
			if (i==1)
				fils1 = ret;
		 i++;
		}  
		else {
			ret = fork ();
			break ;
		}
		
	/* affichage */
	 if (getpid ()!= principal){
		printf ("pid %d; pid pere: %d \n", getpid (),getppid ()); 
	 if ((ret !=0) && (i==2) )
			printf ("pid fils1 :%d\n", fils1);  
	 } 
 
	 if (ret == 0)
		 /* petit-fils */
		 kill (getppid (), SIGUSR1);
	 else 
		if (getpid () != principal) {
	   		/*les deux fils attendent SIGUSR1 de leur fils*/
	    	sigdelset (&sig_proc, SIGUSR1);
	    	sigsuspend (&sig_proc);
	    	if (i==2)/* fils2 - envoie SIGUSR2 au frère (fils1)*/
	       		kill (fils1, SIGUSR2);
	    	else {
	      		/* fils1: attend SIGUSR2 de son frère (fils2)*/
				sigdelset (&sig_proc, SIGUSR2);
	        	sigsuspend (&sig_proc);
				/* envoie SIGUSR2 au processus main */     	
				kill (principal, SIGUSR2);
	    	}	      
		 }else{
			/* le processus main attend SIGUSR2 de son premier fils (fils1) */ 
	    	sigdelset (&sig_proc, SIGUSR2);
	    	sigsuspend (&sig_proc);
		 }  
}