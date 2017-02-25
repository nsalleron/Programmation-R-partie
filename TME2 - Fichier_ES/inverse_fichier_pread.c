#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[]) {
	
	int fd,fd2;
	char c;
	int nbChar;
	
	if(argc < 3)
		return EXIT_FAILURE;
		
	if((fd = open(argv[1],O_RDONLY,NULL)) == -1){
		perror("Problème d'ouverture du fichier\n");
		return EXIT_FAILURE;
	}
	if((fd2 = open(argv[2],O_CREAT|O_RDWR|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO)) == -1){
		perror("Problème d'ouverture du fichier\n");
		return EXIT_FAILURE;
	}
	
	while (read(fd, &c, sizeof(char))>0) 
		nbChar++;
	printf("nombre de caractère : %d\n",nbChar);
	int init = 0;
	while (pread(fd, &c, sizeof(char), init++)>0) {
		pwrite(fd2, &c, sizeof(char), nbChar--);
	}
	
	
}