#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

//Ecrivez un programme un C qui lit sur la ligne de commande deux chemins absolus dans l’arboresence Unix, et teste s’il s’agit du même fichier. 
//On utilisera la fonction stat. Testez votre programme sur différents cas, notamment des liens symboliques ou non symboliques.


int main(int argc, char *argv[]) {
	
	int fd1, fd2;
	struct stat stat1, stat2;
	
	if(argc != 3){
		perror("Problème pour le nombre d'arguments.\n"); return EXIT_FAILURE;
	}
	printf("Argument 1 : %s\n", argv[1]);
	printf("Argument 2 : %s\n", argv[2]);
	
	if(access(argv[1], R_OK) == -1)
		return EXIT_FAILURE;
	if(access(argv[2], R_OK) == -1)
		return EXIT_FAILURE;
	
	if((fd1 = open(argv[1],O_RDWR,0700)) == -1){
		perror("Problème d'ouverture du fichier\n");
		return EXIT_FAILURE;
	}
	if((fd2 = open(argv[2],O_RDWR,0700)) == -1){
		perror("Problème d'ouverture du fichier\n");
		return EXIT_FAILURE;
	}
	
	if (lstat(fd1,&stat1) == -1){
		perror("Problème dans le fstat\n");
	}
	if (lstat(fd2,&stat2) == -1){
		perror("Problème dans le fstat\n");
	}
	
	//if the inodes and device numbers are equal, it is impossible for the two paths to refer to different files
	if(stat1.st_dev == stat2.st_dev && stat1.st_ino == stat2.st_ino){
		printf("Les fichiers partage le même numéro de device et le même numéro d'inode\n");
	}else{
		printf("Les fichiers ne partagent pas le même numéro d'inode et/ou de device\n");
	}
	
	return EXIT_SUCCESS;
}