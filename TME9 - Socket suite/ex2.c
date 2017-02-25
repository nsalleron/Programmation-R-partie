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
int *socketfd, fichierfd, logfd;
char NOM_REPERTOIRE[TAILLE];
struct in_addr ipAddr;
char str[INET_ADDRSTRLEN+1];
fd_set readfds;


int main(int argc, char *argv[]) {
	
	
	int MAXCLIENT = 10;
	int i = 0;
	int max = 0;
	FD_ZERO(&readfds);
	
	if(argc <2){
		fprintf(stderr, "usage : %s <port...> ",argv[0]);
		exit(-1);
	}
	
	/*Allocation de notre tableau de socket */
	socketfd = malloc(sizeof(int)*argc-1);
	
	/* Ouverture du fichier de log */
	if((logfd = open("cx.log", O_CREAT|O_RDWR, 0666)) == -1){
		perror("cx.log");
		exit(2);
	}
	
	for(i = 0; i< argc-1;i++){
		
		printf("Création du socket : %s\n",argv[i+1]);
		/* Création de la socket */
		if((socketfd[i] = socket(AF_INET,SOCK_STREAM,0)) == -1){
			perror("Création socket problème ");
			exit(-1);
		}
		
		printf("Socketfd : %d\n",(int)socketfd[i]);
			
		/* Préparation de l'adresse d'attachement */
		adr.sin_family = AF_INET;
		adr.sin_addr.s_addr = htonl(INADDR_ANY);
		adr.sin_port = htons(atoi(argv[i+1]));
		
		/* Attachement de la socket */
		if (bind(socketfd[i], (struct sockaddr *)&adr,adr_len)==-1){
			perror("Attachement impossible ");
			exit(-1);
		}
		
		/* ouverture du service */
		if(listen(socketfd[i], MAXCLIENT) == -1){
			perror("listen impossible ");
			exit(-1);
		}
		
		/*On ajout à l'ensemble des descripteurs */
		FD_SET(socketfd[i],&readfds);
		printf("-> fin d'initialisation du socket : %d\n",socketfd[i]);
		max = socketfd[i] > max ? socketfd[i] : max;
	}
		
	/* Boucle de traitement */
	while(1){
		
		/*Attente d'une lecture disponible sur l'ensemble */
		if(select(max + 1, &readfds, NULL, NULL, NULL) == -1)
		{
			perror("select");
			exit(errno);
		}
		
		/* On test l'ensemble des descripteurs pour savoir sur lequel répondre */
		for(i = 0;i<argc-1;i++){
			
			if(FD_ISSET(socketfd[i],&readfds)){
				
				printf("Connexion sur socketfd: %d\n",socketfd[i]);
				connexion = accept(socketfd[i], (struct sockaddr *)&adr, &adr_len);
				
				if (connexion == -1) {
					if(errno == EINTR) continue; //Intéruption d'appel, on continue
					else {	//Trop grave, on quitte.
						perror("accept");
						exit(-1);
					}
				}
				
				ipAddr= adr.sin_addr;
				inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );
				str[strlen(str)] = '\n';
				printf("Adresse : %s\n",str);
				write(logfd, str, strlen(str));
				close(connexion);
			}	
		}
		
		FD_ZERO(&readfds);
		
		for(i = 0;i<argc-1;i++)
			FD_SET(socketfd[i],&readfds);
		
		
		
		
	}
		
	
	
}