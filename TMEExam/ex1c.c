#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/*Écrire un programme qui prend en paramètre un nom de fichier et un entier naturel n, et affichant les n premiers caracteres du fichier en ordre inverse. 

Une contrainte est que le programme doit créer n processus, 
chaque caractere devant être lu et émis par un seul de ces processus. 

Les fonctions lseek et pread ne doivent pas être utilisées, et le fichier ne doit être ouvert qu’une fois, par le processus principal. Pour chaque caractère, on affichera également le PID et PPID. */

int main(int argc, char *argv[]){


	int fic; 
	int NBL; //Nombre de lettres
	pid_t prems;
	int i;
	char lu;
	pid_t pid;
	
	
	if(argc!=3){
		fprintf(stderr,"Nb argument incorrect\n");
		exit(EXIT_FAILURE);
	}
	
	NBL = atoi(argv[2]);
	
	/* ouverture du fichier */
	if ((fic = open(argv[1],O_RDONLY))==-1){
			fprintf(stderr,"%s n'existe pas !!\n",argv[1]);
			exit(EXIT_FAILURE);
	}
	
	/* PID du père */
	prems = getpid();
	printf("prems : %d\n", prems);
	
	for(i=0;i<NBL;i++){ // Pour toute les lettres
	
		if(read(fic,&lu,sizeof(char))!=0){	//J'ai correctement lu la lettre.
			
			if((pid=fork())==-1){	//Fork
				fprintf(stderr,"Erreur fork\n");
				exit(EXIT_FAILURE);
			}
			
			if(pid!=0){	//Le père
				
				wait(NULL); //Attente du fils qui va lire la suivante
				printf("%d de pere %d a lu : %c\n",getpid(),getppid(), lu);
				break;
			} 
			//Si le fils est le dernier alors il va sortir de la boucle 
			//et la lettre va ensuite être lu dans l'ordre inverse. INGENIEUX
			
		}else{
			fprintf(stderr,"Erreur lecture\n");
			exit(EXIT_FAILURE);
		}
		
	}
	
	if(prems==getpid())
		close(fic);
}