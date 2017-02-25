#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

//En utlisant la fonction dup2, écrire une fonction Rediriger_stdout redirigeant la sortie standard vers un fichier donné en argument. On considère que le fichier n’existe pas. 
//Ecrire ensuite une deuxième fonction Restaurer_stdout qui restaure la sortie vers le terminal. Ecrire enfin une fonction main prenant en argument un nom de fichier, et qui appelle 3 fois la fonction printf, les deux premiers appels encadrant un appel à Rediriger_stdout sur le fichier indiqué, et les deux derniers un appel à Restaurer_stdout. Qu’observez-vous dans le flux de sortie et le fichier ?
//
//Exemple d'appel :
//$PWD/bin/rediriger trace.txt

int Rediriger_stdout(int desc){
	//Changement de l'adresse de l'écran et placement du descripteur à l'emplacement 1 (normalement adresse de l'écran).
	if ( dup2(desc, 1) == -1)	
		perror("Problème dup2");
	return EXIT_SUCCESS;
}

int Restaurer_stdout(int std_out){
	
	if ( dup2(std_out,1) == -1) //Restauration de l'écran
		perror("Problème dup2");
	return EXIT_SUCCESS;
}


int main(int argc, char *argv[]) {
	
	int fd;
	
	if(argc < 1)
		return EXIT_FAILURE;
	
	if((fd = open(argv[1],O_CREAT|O_RDWR|O_TRUNC,0700))==-1)
		perror("pb ouverture");
		
	int std_old = dup(STDOUT_FILENO); // Duplication de l'adresse de la sortie pour pouvoir la récupérer par la suite.
	
	printf("Apl 1 \n");
	Rediriger_stdout(fd);
	printf("Apl 2\n");
	Restaurer_stdout(std_old);
	printf("Apl 3\n");	
	
	return EXIT_SUCCESS;
	
	
}