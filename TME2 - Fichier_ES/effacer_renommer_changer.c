#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

//Ecrire un programme qui permet d’effacer, de renommer ou de changer les droits d’un fichier existant. Le programme reçoit en argument :
//
//	type d’opération :
//	“E” ou “e” pour effacer
//	“R” ou “r” pour renommer
//	“C” ou “c” pour changer les droits
//	nom du fichier
//	nom du fichier à renommer ou nouveaux droits :
//	“R” ou “r” (read-only / lecture seulement) // + écriture pour le propriétaire du fichier R pour GRP et OTH
//	“W” ou “w” (read-write / lecture-ecriture)	// RW partout
//	Observations :
//
//	Le programme doit vérifier que le deuxième argument n’est pas un répertoire (utiliser la fonction stat).
//	pour le changement de droits en “read-only” n’oubliez pas de donner le droit d’écriture au propriétaire du fichier.

//Exemple d'appel :
//	$PWD/bin/droitsfichier c makefile W


int efface(char *filename){
	printf("je suis dans efface\n");
	//Il faut supprimer le référencement vers le fichier
	if(unlink(filename) == -1){
		perror("La suppression à échoué !\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int renomme(char *nameOld, char *nameNew){
	printf("ancien : %s\n",nameOld);
	printf("nouveau : %s\n",nameNew);
	if(rename(nameOld, nameNew) == -1){
		perror("Le renommage à échoué !\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
	
}

int change(char *filename, char *mode){
	
	int result = 0; 
	if((strcmp(mode, "R") == 0) || (strcmp(mode, "r") == 0))
		result = chmod(filename, S_IREAD|S_IWUSR);
	else if ((strcmp(mode, "W")  == 0)|| (strcmp(mode, "w") == 0))
		result = chmod(filename, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	else {
		printf("Problème argument R/W");
	}
	if(result == -1){
		perror("Problème chmod\n");
		return EXIT_FAILURE;
	}	
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	
	int fd1;
	struct stat stat1;
	
	if(argc == 1)
		return EXIT_FAILURE;
		
	if((fd1 = open(argv[2],O_RDWR,0700)) == -1){
		perror("Problème d'ouverture du fichier\n");
		return EXIT_FAILURE;
	}
	if (fstat(fd1,&stat1) == -1){
		perror("Problème dans le fstat\n");
		return EXIT_FAILURE;
	}
	
	//Vérification si le deuxième argument n'est pas un répertoire
	if(S_ISDIR(stat1.st_mode)){
		perror("C'est un directory\n");
		return EXIT_FAILURE;
	}
	
	if((strcmp(argv[1], "E") == 0) || (strcmp(argv[1], "e") == 0))
		efface(argv[2]);
	
	if ((strcmp(argv[1], "R")  == 0)|| (strcmp(argv[1], "r")  == 0))
		renomme(argv[2],argv[3]);
	
	if ((strcmp(argv[1], "C")  == 0)|| (strcmp(argv[1], "c") == 0))
		change(argv[2],argv[3]);
	
	return EXIT_SUCCESS;
}