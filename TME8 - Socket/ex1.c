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
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>

int main(int argc, char *argv[]){

	int id, n, i, total, res;
	int *result = NULL;
	int *pids;
	struct sockaddr_un si_me, si_other;
	socklen_t lenslt = (unsigned) sizeof(struct sockaddr_un);
	
	n = (argc < 2) ? 0 : strtol(argv[2], NULL, 10); //argc inférieur à 2? oui n = 0 non n = strtol
	
	if (n <= 0) {
		fprintf(stderr, "Usage: %s nombre\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	unlink(argv[1]);
	
	/* Créatiion descripteur */
	if ((id = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
		close(id);
		perror("Echec de création de la socket\n");
		exit(errno);
	}
	
	/* Bind le nom au socket */
	memset(&si_me,'\0',sizeof(struct sockaddr_un));
	si_me.sun_family = AF_UNIX;
	strcpy (si_me.sun_path, argv[1]);
	
	if (bind(id,(struct sockaddr *) &si_me, sizeof(si_me))==-1){
		close(id);
		perror("Echec du bind\n");
		exit(errno);
	}
	
	pids = malloc(n * sizeof(int));
	result = malloc(n * sizeof(int));
	
	for(i=0; i<n; i++){
		
		int pid = fork();
		
		if (pid == -1){
			
			perror("fork");
			return -1;
			
		} else if (pid) {
			
			pids[i] = pid;
			int val;
			
			if (recvfrom(id, &val, sizeof(int), 0,(struct sockaddr *)&si_me, &lenslt)==-1){
				//close(id);
				perror("père : Echec rcv");
				exit(errno);
			}
			result[i] = val;
			printf("père val : %d\n",val);
			val+=1;
			
			printf("fin i=%d\n",i);
		} else {
			srand(time(NULL)*i);
			/* Ecriture puis lecture dans la socket */
			int val = (int) (10*(float)rand()/ RAND_MAX);
			printf("%d | val : %d\n",getpid(),val);

			if (sendto(id, &val, sizeof(int), 0,(struct sockaddr *)&si_me, lenslt ) == -1) {
				perror("fils sending datagram message");
				exit(errno);
			}
		
			printf("%d | val reçu : %d\n",getpid(),val);
			
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
		res = result[i];
		printf("pid %d envoie %d\n", pids[i], res);
		total += res;
	}

	free(pids);

	printf("total: %d\n", total);
	return EXIT_SUCCESS;
}
