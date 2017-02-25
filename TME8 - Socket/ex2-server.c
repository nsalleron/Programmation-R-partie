#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define BUFSIZE 1024

int main(int argc, char **argv) {
	
	int sockfd; /* socket */
	int portno; /* port d'écoute */
	
	
	struct sockaddr_in serveraddr; /* server's addr */
	socklen_t servlen = sizeof(serveraddr); /* taille de l'adresse client */
	struct hostent *hostp; /* client host info */
	char buf[BUFSIZE]; /* message buf */
	char *hostaddrp; /* dotted decimal host addr string */
	int optval; /* flag value for setsockopt */
	int n; /* message byte size */
	char *REP = "DONE";
	char *USER;
	char *VARIABLE;
	/* 
	 * Vérification ligne de commande
	 */
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	portno = atoi(argv[1]);

	/* 
	 * socket: création de la socket
	 */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
		perror("socket ");


	/*
	 * préparation de l'adresse d'attachement
	 */

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);

	/* 
	 * bind: attachement de la socket
	 */
	if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr_in)) < 0) 
		perror("bind");
	
	printf("Socket UDP sur port : %d\n",ntohs(serveraddr.sin_port));

	/* 
	 * boucle
	 */
	while (1) {

		/*
		 * recvfrom: réception d'un datagramme UDP d'un client
		 */
		printf("Caractères dans le buff: %lu\n",strlen(buf));
		bzero(buf, BUFSIZE);
		
		printf("-> Vidage, caractères dans le buff: %lu\n",strlen(buf));
		n = recvfrom(sockfd, buf, BUFSIZE, 0,(struct sockaddr *) &serveraddr, &servlen);
		printf("Caractères dans le buff avant traitement: %lu\n",strlen(buf));

		if (n < 0)
			perror("recvfrom ");

		
		//Traitement du message
		char *token = strtok(buf, " ");
		printf("token : %s\n",token);
		if(!strcmp(token, "S")){
			
			USER = strtok(NULL, " ");
			VARIABLE = strtok(NULL, " ");
			setenv(USER, VARIABLE, 1);
			n = sendto(sockfd, REP, strlen(REP), 0, (struct sockaddr *) &serveraddr, servlen);
				if (n < 0) 
					perror("sendto");
		
			printf("Variable env : %s\n",getenv(USER));
			
					
		}
		if (!strcmp(token, "G")){
				
			USER = strtok(NULL, " ");
			USER[strlen(USER)-1]= 0;		//On retire le caractère \n de l'exemple
			VARIABLE = getenv(USER);
			if(strlen(VARIABLE)!=0)
				printf("VARIABLE : %s\n",VARIABLE);
			n = sendto(sockfd, VARIABLE, strlen(VARIABLE), 0, (struct sockaddr *) &serveraddr, servlen);
			if (n < 0) 
				perror("sendto");
			
		}
		if (!strcmp(token, "Q\n")){
			n = sendto(sockfd, REP, strlen(REP), 0, (struct sockaddr *) &serveraddr, servlen);
				if (n < 0) 
					perror("sendto");
		}
		
		
		
	}
}