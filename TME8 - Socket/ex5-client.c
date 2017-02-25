#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#define TAILLE 256

int main(int argc, char *argv[]) {
	
	
	struct sockaddr_in adr;
	socklen_t adr_len = sizeof(struct sockaddr_in);
	int connexion, n, port;
	int socketfd, fichierfd;
	struct hostent *hp; // pour l'adresse du serveur
	char NOM_FICHIER[TAILLE];
	char DATA[4*TAILLE];
	if(argc <4){
		fprintf(stderr, "usage : %s <addr> <port> <filename>",argv[0]);
		exit(-1);
	}
	
	/* recherche de l'adresse Internet du serveur */
	
	if((hp = gethostbyname(argv[1]))== NULL){
		fprintf(stderr, "machine %s incconue \n",argv[1]);
		exit(2);
	}
		
		
	/* Création de la socket */
	if((socketfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("Création socket problème ");
		exit(-1);
	}
		
	/* Préparation de l'adresse d'attachement */
	adr.sin_family = AF_INET;
	memcpy(&adr.sin_addr.s_addr, hp->h_addr, hp->h_length);
	adr.sin_port = htons(atoi(argv[2]));
	
	
	/* ouverture du service */
	if(connect(socketfd, (struct sockaddr *)&adr,adr_len) == -1){
		perror("connection impossible ");
		exit(-1);
	}
	
	fprintf(stdout, "Connexion réussi\n");
	
	/*Ouverture du fichier */
	fichierfd = open(argv[3], O_RDONLY, 0666);
	
	/*Vidage et copie dans le buffer pour le nom */
	memset(NOM_FICHIER, 0, TAILLE);
	strcpy(NOM_FICHIER, argv[3]);
	
	/*Ecriture sur la socket*/
	//n = write(socketfd, NOM_FICHIER, TAILLE);
	n = write(socketfd, "LIST ", sizeof("LIST "));
	if (n < 0) 
		perror("ERROR in sendto");
	//sleep(1);
	/* Boucle de traitement */
	//while((n=read(fichierfd,&DATA,4*TAILLE)) != 0){
	while((n=read(socketfd,&DATA,4*TAILLE)) != 0){
		printf("%s",DATA);
		//n = write(socketfd, DATA, 4*TAILLE);
		if (n < 0) 
			perror("ERROR in sendto");
			
		/* Ne pas oublier de remettre à 0 le buffer */
		memset(DATA, 0, 4*TAILLE);
	}
	shutdown(socketfd, SHUT_RDWR);
	close(socketfd);
	close(fichierfd);
		
	
	
}