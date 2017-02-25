#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc,char *argv[]){
	pid_t fils[2];
	int i, j , status;
	printf("Père : %d\n",getpid());
	for(i = 0;i<2;i++){
		int valFork;                
		if((fils[i] = fork()) == -1){
			perror("fork");
			exit(-1);
		}

		if(fils[i] == 0){ // Les fils
			if(i == 1)        //fils 2
				printf("Fils 2 \tMon PID : %d, PPID : %d, PID de mon frere : %d\n",getpid(),getppid(), fils[0]);
			else                //fils 1
				printf("Fils 1 \tMon PID : %d, PPID : %d\n",getpid(),getppid());
		
			pid_t pfils;        
			if((pfils = fork()) == -1){
				perror("fork");
				exit(-1);
			}
			if(pfils == 0){        //les petits fils
				
				printf("Fils %d.1 - Mon PID : %d, PPID : %d\n",i+1,getpid(),getppid());
				exit(1);
			}else{
				waitpid(fils[i], &status, NULL);        
				printf("Mon petit fils est mort\n");        
				exit(1);                
			}
		}
	}//for
	//sleep(1);
	for(j = 0;j<2;j++){
		
		waitpid(fils[j],&status,NULL);
		printf("Mon fils %d est mort\n", fils[j]);        
	}
}//main