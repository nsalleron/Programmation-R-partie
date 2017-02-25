#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


//Écrire en C deux fonctions qui lisent caractère par caractère un fichier passé en argument, et qui affichent chaque caractère dès qu’il est lu. Ces fonctions utiliseront 3 processus partageant le même descripteur et agissant à l’identique (l’identité de chaque processus est affichée avec le caractère lu). La première utilisera les standard POSIX open et read, la seconde fopen et fgetc. Vous écrirez un unique programme qui, selon que son premier argument est -p ou -C appliquera l’une ou l’autre de ces fonctions sur le fichier donné en 2e argument. Vous contrôlerez les cas d’erreur comme précédent. Quelle différence observez-vous entre les deux modes d’appels ?
//
//Exemple d'appel :
//$PWD/bin/lectures -p src/lectures.c; $PWD/bin/lectures -C src/lectures.c

int c(char *file){
	FILE* ptFile;
	pid_t fils;
	char c;

	if((ptFile = fopen(file, "r+")) == NULL){
		perror("-> Problème ouverture"); return EXIT_FAILURE;
	}
	for (int i = 0;i<3;i++) {
		
			if((fils = fork()) ==-1){
				perror("fork"); exit(EXIT_FAILURE);
			}else if(fils ==0){        //Dans le fils
				while ((c=fgetc(ptFile))!= EOF) {
						printf("Moi : %d / Père %d LIBC : %c\n",getpid(), getppid(),c);
				}
				return EXIT_SUCCESS;
			}
	}
	return EXIT_SUCCESS;
	
}

int p(char *file){
	int fd;
	char c;
	pid_t fils;
	
	if((fd = open(file, O_RDONLY,0600)) == -1){
		perror("-> Problème ouverture"); return EXIT_FAILURE;
	}
	for (int i = 0;i<3;i++) {
		if((fils = fork()) ==-1){
			perror("fork"); exit(EXIT_FAILURE);
		}else if(fils ==0){        //Dans le fils
			while (read(fd, &c, sizeof(char))>0) {
				printf("Moi : %d / Père %d POSIX : %c\n",getpid(),getppid(),c);
			}
			return EXIT_SUCCESS;
		}
	}
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	
	if(argc != 3){
		perror("Pas assez d'arguments.");
		return EXIT_FAILURE;
	}
	
	if(strcmp(argv[1], "-p") == 0){
		printf("Passage mode POSIX\n");
		p(argv[2]);
		
	}else if(strcmp(argv[1], "-C") == 0){
		printf("Passage mode C\n");
		c(argv[2]);
	}else {
		printf("Argument incorrect.\n");
		exit(EXIT_FAILURE);
	}
	
}