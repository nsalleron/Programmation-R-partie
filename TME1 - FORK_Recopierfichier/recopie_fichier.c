#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

//Ecrire en C un programme qui prend en argument deux noms de fichier et recopie intégralement le contenu du premier dans le second, en utilisant les fonctions POSIX open, read et write. On donnera au deuxième fichier les droits en lecture et écriture pour soi (à l’aide du 3e argument de open ou en appelant la fonction chmod). On veillera à dénoncer (avec la fonction perror) les cas d’erreur suivants :
//
//	la ligne de commande ne contient pas exactement 2 noms ;
//	le premier nom ne désigne pas un fichier régulier et accessible en lecture ;
//	le second ne peut être créé (répertoire inaccessible en écriture, ou entrée déjà existante dedans).
//	En cas de réussiste, le programme 0 sinon il retourne la valeur de errno.
//
//	Exemple d'appel :
//	$PWD/bin/mycp src/mycp.c cp.c

int main(int argc,char *argv[]){

	struct stat buf;
	int fd1,fd2;
	char buffer[1];
	int k = 1;
	int n;

	printf("Argc %d\n",argc);
	if(argc != 3){
		printf("Il manque des arguments\n");
		exit(1);
	}
	
	printf("Récupération stat\n");
	
	//Récupération des informations du fichier
	if(stat(argv[1],&buf)!=0){
		perror("Echec stat : ");
		exit(0);        
	}

	printf("Vérification si le fichier est régulier\n");
	//Vérification si le fichier est un fichier régulier
	if(S_ISREG(buf.st_mode)){
		printf("Le fichier est regulier\n");
	}else{
		printf("Le fichier n'est pas un fichier régulier, sortie..\n");
		exit(0);
	}
	printf("Fin de vérification\n");
	//Ouverture du premier fichier
	if((fd1=open(argv[1],O_RDONLY,buf.st_mode)) ==-1){
		printf("Probleme lecture\n");
		printf("%s",strerror(errno));        
	}
	//Ouverture ou création du second fichier
	if((fd2=open(argv[2],O_CREAT, buf.st_mode)) == -1){
		printf("Problème dans la création\n");
		printf("%s\n",strerror(errno));
	}
	
	//Recopie des fichiers
	while((n=read(fd1,&buffer,k)) != -1){
		write(fd2,&buffer, k);
	}

	//Fermeture des fichiers
	close(fd1);
	close(fd2);



}