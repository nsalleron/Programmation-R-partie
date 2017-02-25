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


//Écrire un programme qui prend en paramètre un nom de fichier et un entier naturel n, et affichant les n premiers caractères du fichier en ordre inverse. OK
//Une contrainte est que le programme doit créer n processus, chaque caractère devant être lu et émis par un seul de ces processus. 
//Les fonctions lseek et pread ne doivent pas être utilisées, et le fichier ne doit être ouvert qu’une fois, par le processus principal. Pour chaque caractère, on affichera également le PID et PPID.

int main(int argc, char *argv[]) {
	
	int fd,fd2;
	char c;
	int nbChar;
	
	if(argc < 3)
		return EXIT_FAILURE;
	
	/* Ouverture du premier fichier */
	if((fd = open(argv[1],O_RDONLY,0700)) == -1){
		perror("Problème d'ouverture du fichier\n");
		return EXIT_FAILURE;
	}

	// n premier caractères
	int n = atoi(argv[2]);
	int a = n;
	int i = 0;
	
	//Le premier processus affiche le premier caractère;
	pread(fd, &c, sizeof(char),n);
	printf("%c",c);
	fflush(stdout);
	n--;
	
	for(i = 0;i<a;i++){
		if(fork() == 0){//fils
			if(pread(fd, &c, sizeof(char),n) == -1)
				perror("Read...");
			printf("%c",c);
			break;
		}else {
			wait(0);
			n--;
		}
	}
	fflush(stdout);
	
}