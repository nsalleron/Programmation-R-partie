#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <aio.h>

//1 Asynchronisme avec notification
//
//Ecrire un programme qui prend en argument un nom de fichier et une chaîne de caractères, crée un fichier vide à partir du nom donné en argument, puis écrit la chaîne de caractères dans le fichier avec aio_write, de sorte que la fin de cet appel asynchrone soit notifiée par un signal SIGRTMIN. Pendant l’écriture le programme crée un nouveau descripteur vers le même fichier, puis attend la fin de l’écriture pour aller lire le contenu du fichier et l’affiche avant de se terminer.

char buff[1024];

void real_time_handler(int sig_number, siginfo_t * info,
                       void * arg __attribute__ ((unused)))
{
  	printf("passage\n");
}

int main(int argc, char *argv[]) {
	
	if(argc < 3){
		perror("arg");
		exit(EXIT_FAILURE);
	}
	struct sigaction action;
	sigset_t mask;

	action.sa_sigaction = real_time_handler;
	action.sa_flags = 0;
	sigfillset(&action.sa_mask);
	sigfillset(&mask);

	sigaction(SIGRTMIN,&action,NULL);
	sigprocmask(SIG_BLOCK,&mask,NULL);

	int fd = open(argv[1], O_CREAT|O_WRONLY, 0666);
	struct aiocb *aio;
	aio = malloc(sizeof(struct aiocb));

	strcat(buff,argv[2]);

	printf("strlen : %d\n",strlen(buff));
	//buff[strlen(buff)] = '\n';
	aio->aio_fildes = fd;
	aio->aio_buf = buff;
	aio->aio_nbytes = strlen(buff);
	aio->aio_offset = 0;
	aio->aio_reqprio = 0;
	aio->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	aio->aio_sigevent.sigev_signo = SIGRTMIN;
	
	if (aio_write(aio) == -1) {
		printf(" Error at aio_write(): %s\n", strerror(errno));
		close(fd);
		exit(2);
	}

	sigdelset (&mask, SIGRTMIN);
	sigsuspend(&mask);

	printf("fin sig\n");
	int fd2 = open(argv[2], O_CREAT|O_WRONLY, 0666);
	

	
	read(fd2,buff,1024);
	printf("fin read : %s\n",buff);
	
}
