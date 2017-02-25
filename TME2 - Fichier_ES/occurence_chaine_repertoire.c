#include <stdio.h>
#define _XOPEN_SOUCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

extern int errno;

#define _POSIX_SOUCE 1
#define TAILLE_PATH 100

//
//On considère le fichier liste-rep.c founi en annexe qui liste le contenu d’un répertoire. Ecrire une variante de ce programme qui cherche la chaîne donnée en premier argument dans tous les fichiers du répertoire donné en 2e argument. Il affiche le nom de chaque fichier qui contient la chaîne de caractères recherchée, ou "Aucun fichier valide" si la chaîne n’est présente dans aucun des fichiers du répertoire.
//
//N.B : Vous pouvez utiliser la fonction strstr de la bibliothèque string.h pour trouver si une chaîne de caractères est présente dans une autre.
//
//Exemple d'appel :
//bin/extended-grep if src



char buff_path [TAILLE_PATH];
DIR *pt_Dir;
struct dirent* dirEnt;

int main (int argc, char* argv []) {
	if (argc ==  3) {
		//Répertoire donné en 2ème argument.
		memcpy (buff_path,argv[2],strlen(argv[2]));
	}else
		return EXIT_FAILURE;
		
	if ( ( pt_Dir = opendir (buff_path) ) == NULL) {
		if (errno ==  ENOENT ) {
			/* repertoire n'existe pas - créer le répertoire */
			if (mkdir ( buff_path, S_IRUSR|S_IWUSR|S_IXUSR) == -1) {
				perror ("erreur mkdir\n");
				exit (1);
			}else
				return 0;
		}
		else {
			perror ("erreur opendir \n");
			exit (1);
		}
	}
	
	char *ret;
	int i = 0;
	
	/* lire répertoire */
	while ((dirEnt = readdir (pt_Dir)) != NULL) {
		
		/* Recherche d'une occurence de la chaine rechercher pour l'entrée dans le répertoire. */
		ret = strstr(dirEnt->d_name, argv[1]); 
		
		
		if(ret != NULL){
			printf ("%s\n", dirEnt->d_name);
			i++;
		}
			
	}
	
	/* Dans le cas ou il n'y a pas de chaines */
	if(i==0)
		printf("Aucun fichier correspondant à \"%s\" dans le répertoire.",argv[1]);
	
	//Fermeture
	closedir (pt_Dir);
	
	return EXIT_SUCCESS;
}
