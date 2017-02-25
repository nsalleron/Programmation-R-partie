#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
int main(int argc, char *argv[]){

	int id, n, i, total, res;
	int *ptr = NULL;
	int *pids;
	mqd_t mqdesc;
	struct mq_attr attr;

	mq_unlink("./file");

	ptr = malloc(n*sizeof(int));
	
	
	n = (argc < 2) ? 0 : strtol(argv[1], NULL, 10);
	if (n <= 0) {
		fprintf(stderr, "Usage: %s nombre\n", argv[0]);
		exit(EXIT_FAILURE);
	}
		
	if ((mqdesc = mq_open("./file", O_RDWR|O_CREAT, 0666,&mqattr) == -1) {
		perror("Echec de l'allocation du descripteur\n");
		exit(errno);
	}
	
	
	
	for(i=0; i<n; i++){
		int pid = fork();
		if (pid == -1){
			perror("fork");
			return -1;
		} else if (pid) {
			if (mq_getattr(mqdesc, & attr) != 0) {
				perror("mq_getattr");
				exit(EXIT_FAILURE);
			}
			taille = attr.mq_msgsize;
			mq_receive(mqdesc,&ptr[i], taille, NULL);
			total += *ptr;
			pids[i] = pid;
			printf("Message : %d\n",*ptr);
		} else {
			srand(time(NULL)*i);
			/* Ecriture dans la file */
			mq_send(mqdesc,(int) (10*(float)rand()/ RAND_MAX),sizeof(int), 0);
			//ptr[i] = (int) (10*(float)rand()/ RAND_MAX);
			exit(EXIT_SUCCESS);
		}
	}
	for(i=0; i<n; i++){
		int status;
		waitpid(pids[i], &status, 0);
	}

	total = 0;

	for(i=0; i<n; i++){
		/* Lecture dans la ressource partagee */
		res = ptr[i];
		printf("pid %d envoie %d\n", pids[i], ptr[i]);
	}

	free(pids);

	printf("total: %d\n", total);
	
	

	return EXIT_SUCCESS;
}
