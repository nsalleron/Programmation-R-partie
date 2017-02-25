#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

//Ecrire un programme qui prend en argument un nom de fichier, le lit caractère par caractère pour l’écrire de manière inversée dans un autre fichier. Votre programme doit impérativement utiliser la fonction lseek pour modifier l’offset lors de la lecture.
//
//Si le premier fichier contient "fichier ok" alors le deuxième devra contenir "ko reihcif" (on ne testera évidemment pas sur un fichier contenant un palindrome comme radar etc).
//
//Exemple d'appel :
//bin/inverser-fichier src/inverser-fichier.c

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
	
	/* Ouverture du fichier de destination */
	if((fd2 = open(argv[2],O_CREAT|O_RDWR|O_TRUNC,0700)) == -1){
		perror("Problème d'ouverture du fichier\n");
		return EXIT_FAILURE;
	}
	
	/* On compte le nombre de caractère */
	while (read(fd, &c, sizeof(char))>0) 
		nbChar++;
	
	/* Placement au début */
	printf("nombre de caractère : %d\n",nbChar);
	lseek(fd, 0, SEEK_SET);	//Replacement au début
	lseek(fd2, nbChar, SEEK_SET);	//Placement au nbChar octet à partir du début. Fichier à troue;
	while (read(fd, &c, sizeof(char))>0){	//Tant qu'il reste des octets à lire
		printf("%c",c);
		write(fd2, &c, sizeof(char));	//Ecriture emplacement 
		lseek(fd2, --nbChar, SEEK_SET);	//Décrément
	}
	
	
	
}