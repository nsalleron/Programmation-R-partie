#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#define TAILLE_LIGNE 250
#define TAILLE_ARG 40
#define EVER (;;)

char commande [TAILLE_LIGNE];

int main(int argc, char *argv[]) {
char *arg[TAILLE_ARG];
char *bg;
char *mot;
int i,pid,CodeRetour;
	for EVER {
		printf("> ");
		fgets(commande, TAILLE_LIGNE, stdin);
		commande[strlen(commande)-1]='\0';
		if(bg=strrchr(commande, '&')){
			*bg='\0';}
		for(i=0,mot=strtok(commande," ");mot!=NULL;mot=strtok(NULL," "),i++){
			arg[i]=(char *)malloc(strlen(mot)+1);
			strcpy(arg[i],mot);
		}
		arg[i]=NULL;
		//Nettoyage des précédentes commandes
		//waitpid(-1,&CodeRetour,WNOHANG);
		if(i>0)
			if(fork()==0){
				pid = fork();
				if(pid==0){
					execvp(arg[0],arg);
					perror("execvp");
					exit(1);
				}else {	//Comme son père est fini il est rattaché au processus init
					exit(0);
				}
				
			}
			else{
				if(bg!=NULL){
					waitpid(pid, &CodeRetour, 0);
					printf("Code de retour & : %d\n",CodeRetour);
				}else{
					pid=wait(&CodeRetour);
					printf("Code de retour : %d\n",  CodeRetour);
				}//if(bg!=NULL)
				
			}//if(fork()==0)
	
	}//for EVER
		
		
		
}
