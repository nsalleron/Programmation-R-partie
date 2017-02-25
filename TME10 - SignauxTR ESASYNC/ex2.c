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

//2 Asynchronisme avec suspension

//Modifier le programme de l’exercice précédant pour que la fin de l’écriture ne soit plus notifiée par signal, mais soit attendue par un appel à aio_suspend. Pendant l’écriture le programme crée un nouveau descripteur vers le même fichier, puis attend la fin de l’écriture pour aller lire, cette fois-ci de manière asynchrone (aio_read), le contenu du fichier et l’affiche avant de se terminer.

char buff[1024];


int main(int argc, char *argv[]) {
	
	if(argc < 3){
		perror("arg");
		exit(EXIT_FAILURE);
	}

	int fd = open(argv[1], O_CREAT|O_WRONLY, 0666);
	struct aiocb aio;
	//aio = malloc(sizeof(struct aiocb));

	struct aiocb *lio[1];

	strcat(buff,argv[2]);

	printf("strlen : %d\n",strlen(buff));
	//buff[strlen(buff)] = '\n';
	aio.aio_fildes = fd;
	aio.aio_buf = buff;
	aio.aio_nbytes = strlen(buff);
	aio.aio_offset = 0;
	aio.aio_reqprio = 0;
	aio.aio_sigevent.sigev_notify = SIGEV_NONE;
	aio.aio_sigevent.sigev_signo = SIGRTMIN;
	
	if (aio_write(&aio) == -1) {
		printf(" Error at aio_write(): %s\n", strerror(errno));
		close(fd);
		exit(2);
	}
	
	lio[0] = &aio;

	if(aio_suspend(lio,1,NULL) == -1){
		perror("aio_suspend ");
		exit(-1);	
	}

	printf("fin sig\n");
	int fd2 = open(argv[2], O_CREAT|O_WRONLY, 0666);
	

	
	read(fd2,buff,1024);
	printf("fin read : %s\n",buff);
	
}
