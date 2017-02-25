#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
//
//Ecrire un programme qui reçoit au moins trois arguments :
//
//	un nom de fichier à créer ;
//	un mot quelconque ;
//	une suite de mots quelconques.

//	Le programme doit créer le fichier et écrire la suite de mots dans le fichier, ainsi que dans le flux de sortie. 
//	Ce même fichier est ensuite parcouru en utilisant la fonction read et la lecture s’arrête après le premier mot. 
//	On remplace alors le deuxième mot de la suite par le mot donné en deuxième argument. 
//	On suppose que le remplaçant et le remplacé sont de même longueur.
//
//	Exemple d'appel :
//	$PWD/bin/remplacedansfichier texte toi a moi de jouer

int main(int argc, char *argv[]) {
	
	int fd;
	
	if(argc < 3)
		return EXIT_FAILURE;
	
	if((fd = open(argv[1],O_CREAT|O_RDWR|O_TRUNC,0700)) == -1){
		perror("Problème d'ouverture du fichier\n");
		return EXIT_FAILURE;
	}
	
	int i = 3; // On commence l'affichage du texte en même temps que l'écriture.
	
	while (argv[i] != NULL) {
		
		if(write(fd, argv[i], strlen(argv[i])) == -1){
			perror("Probleme write");
			return EXIT_FAILURE;
		}
		printf("%s ",argv[i]);
		
		if(write(fd, " ", strlen(" ")) == -1){
			perror("Probleme write");
			return EXIT_FAILURE;
		}
		i++;
	}
	printf("\n");
	
	lseek(fd, 0, SEEK_SET);
	char c;
	while (read(fd, &c, sizeof(char))>0) {
		if(c == ' ' ){
			if(write(fd, argv[2], strlen(argv[2])) == -1){
				perror("Write pb ");
			}
			break;
		}
	}
}