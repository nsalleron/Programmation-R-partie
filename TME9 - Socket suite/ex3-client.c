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
#include <pthread.h>
#include <arpa/inet.h>

#define TAILLE 256
struct sigaction action;
pthread_t pidThread;
struct sockaddr_in adr;
socklen_t adr_len = sizeof(struct sockaddr_in);
int connexion, n;
int *connexionToThread;
int socketfd,socketfdEnvoi, fichierfd, logfd;
char NOM_REPERTOIRE[TAILLE];
struct in_addr ipAddr;
char str[INET_ADDRSTRLEN+1];
fd_set readfds;


int main(int argc, char *argv[]) {
	
	
	int MAXCLIENT = 10;
	int i = 0;
	int max = 0;
	int broadcastPermission = 1;
	
	//if(argc <2){
		//fprintf(stderr, "usage : %s <ip> <port> ",argv[0]);
		//exit(-1);
	//}
	
	//broadcastIP = 127.0.0.1;//argv[1];            /* Adresse IP*/ 
	int broadcastPort = 9999;//atoi(argv[2]);    /* Port pour broadcast */
	char *sendString = "PING";//argv[3];             /* Message */
	char *rcvString;
	rcvString = malloc(sizeof(char)*4);
	/* Création de la socket */
	if((socketfd = socket(AF_INET,SOCK_DGRAM,0)) == -1){
		perror("Création socket problème ");
		exit(-1);
	}
		
	/* Préparation de l'adresse d'attachement */
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = htonl(INADDR_ANY);
	adr.sin_port = htons(broadcastPort);
	
	
	/* Attachement au port */
	if (bind(socketfd, (struct sockaddr *) &adr, sizeof(adr)) < 0)
		perror("bind");
	
	int adrlen = sizeof(adr);
		
	/* Boucle de traitement */
	while(1){
		read(socketfd, rcvString, strlen(sendString));
		printf("RECU : %s\n",rcvString);
		
		if(!strcmp(rcvString,"PING")){
			printf("ENVOI : PONG\n");
			if (sendto(socketfdEnvoi, "PONG", strlen("PONG"), 0, (struct sockaddr *) &adr, sizeof(adr)) != strlen("PING"))
			    perror("sendto");
		}
		sleep(1);
	}
		
	
	
}