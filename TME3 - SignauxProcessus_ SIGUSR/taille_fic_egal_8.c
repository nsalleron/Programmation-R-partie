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

//Ajouter à ce programme l’envoi d’un seul signal de sorte que la taille du fichier soit égale à 8 dans tous les scénarios d’exécution.

void sigTrt(int sig){
	if(sig == SIGUSR1)
		printf("%d | Je peux écrire.\n", getpid());
}

int main (void) {
		pid_t fils;
		int fd1, fd2, fd3;
		
		if ((fd1 = open ("./fich1", O_RDWR| O_CREAT | O_TRUNC, 0600)) == -1) {
			perror("./fich1");
			return errno;
		}
		if (write (fd1,"abcde", strlen ("abcde")) == -1) {
			perror("./fich1");
			return errno;
		}
		
		
		struct sigaction action;
		sigset_t ens_sig;
		
		sigfillset(&ens_sig);
		sigdelset(&ens_sig, SIGUSR1);
		
		action.sa_mask = ens_sig;
		action.sa_flags = 0;
		action.sa_handler = sigTrt;
		sigaction(SIGUSR1,&action,NULL);
		
		
		//Taille = 5
		
		if ((fils = fork ()) == 0) {
				if ((fd2 = open ("./fich1", O_RDWR)) == -1) {
	  				perror("./fich1");
	 				return errno;
				}
				if (write (fd1,"123", strlen ("123")) == -1) {
	  				perror("./fich1");
	  				return errno;
				}
				//Taille = 8
				
				//Information pour le père, il peut écrire
				kill(getppid(),SIGUSR1);
				sigsuspend(&ens_sig);
				
				if (write (fd2,"45", strlen ("45")) == -1) {
	  				perror("./fich1");
	  				return errno;
				}
				close(fd2); 
		} else {
			
			
				fd3 = dup(fd1);
				
				//attente écriture du fils
				sigsuspend(&ens_sig);
				//Il peut maintenant écrire
				
				//RAZ 0
				if (lseek (fd3,0,SEEK_SET) == -1) {
	  				perror("./fich1");
	  				return errno;
				}
				
				
				//fg
				if (write (fd3,"fg", strlen ("fg")) == -1) {
	  				perror("./fich1");
	  				return errno;
				}
				if (write (fd1,"hi", strlen ("hi")) == -1)  {
	  				perror("./fich1");
	  				return errno;
				}
				
				kill(fils, SIGUSR1);
				
				
				wait (NULL);
				close (fd1);
				close(fd3);
				return EXIT_SUCCESS;
		}
}