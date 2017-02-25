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
pthread_t pidEnvoi, pidEcoute;
struct sockaddr_in adr,adrEcoute;
socklen_t adr_len = sizeof(struct sockaddr_in);
int connexion, n;
int *connexionToThread;
int socketfdEnvoi, socketfdEcoute, fichierfd, logfd;
char NOM_REPERTOIRE[TAILLE];
struct in_addr ipAddr;
char str[INET_ADDRSTRLEN+1];
fd_set readfds;
char *rcvString;
int broadcastPort = 9999;//atoi(argv[2]);    /* Port pour broadcast */
char *sendString = "PING";//argv[3];             /* Message */
int broadcastPermission = 1;
void funcEnvoi(void * arg){
	printf("Création du thread\n");
	
	
	/* Création de la socket */
	if((socketfdEnvoi = socket(AF_INET,SOCK_DGRAM,0)) == -1){
		perror("Création socket problème ");
		exit(-1);
	}
	
	/* Set socket to allow broadcast */
	if (setsockopt(socketfdEnvoi, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, 
		  sizeof(broadcastPermission)) < 0)
		perror("setsockopt");
	
		
	/* Préparation de l'adresse d'attachement */
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	adr.sin_port = htons(broadcastPort);
		
	/* Boucle de traitement */
	while(1){
		if (sendto(socketfdEnvoi, sendString, strlen(sendString), 0, (struct sockaddr *) &adr, sizeof(adr)) != strlen("PING"))
		    perror("sendto");
		sleep(1);
	}
	
}

void funcEcoute(void* arg){

	/* Création de la socket */
	if((socketfdEnvoi = socket(AF_INET,SOCK_DGRAM,0)) == -1){
		perror("Création socket problème ");
		exit(-1);
	}
	
	/* Set socket to allow broadcast */
	if (setsockopt(socketfdEnvoi, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, 
		  sizeof(broadcastPermission)) < 0)
		perror("setsockopt");
	
		
	/* Préparation de l'adresse d'attachement */
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	adr.sin_port = htons(broadcastPort);
	while(1){
		read(socketfdEcoute, &rcvString, strlen("PING"));
		printf("THREAD ECOUTE | Recu : %s\n",rcvString);
	}

}




int main(int argc, char *argv[]) {
	
	
	rcvString = malloc(sizeof(char)*4);
	pthread_create(&pidEnvoi, 0, (void*)funcEnvoi, (void*)0);
	pthread_create(&pidEcoute, 0, (void*)funcEcoute, (void*)0);
		
	pthread_join(pidEnvoi,(void*) 0);
	pthread_join(pidEcoute, (void*) 0);
	
	
}