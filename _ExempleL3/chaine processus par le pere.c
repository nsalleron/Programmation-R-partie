#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>

int main(int argc, char *argv[]) {
	int ato=atoi(argv[1]);
	//printf("ato:%d\n",ato);
	pid_t pid, origpid = getpid();
	int status;
	int nbprocess =0;
	printf("Je suis le pere: %d\n",getpid());
	while (nbprocess!=ato) {
		//printf("nbprocess : %d\n",nbprocess+1);
		nbprocess+=1;
		pid = fork();
		if(pid==0){
			printf("le ");
			for (int i = 1; i < nbprocess; i++) {
				printf("petit ");
			}
			printf("fils %d dodo\n",getpid());
			//sleep(nbprocess);
		}else {
			break;
		}
	}
	if(pid!=0){
		waitpid(pid, &status, 0);
		printf("fils %d meurt \n=> valeur : %d\n",pid,WEXITSTATUS(status));
	}else{
		printf("=> JE SUIS LE DERNIER PETIT :(\n");
	}
	if(origpid==getpid()){
		printf("le papa %d meurt",getpid());
		exit(EXIT_SUCCESS);
	}else
		exit(nbprocess);
}