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
	char *CMD;
	char CMD_BRUT[TAILLE];
	char CMD_COMPLET[TAILLE];
	
	char *REP_COURANT = "./";
	char *NOM_EXTRAIT;
	char DATA[4*TAILLE];
	if(argc <3){
		fprintf(stderr, "usage : %s <addr> <port> <filename>",argv[0]);
		exit(-1);
	}
	
	/* recherche de l'adresse Internet du serveur */
	
	if((hp = gethostbyname(argv[1]))== NULL){
		fprintf(stderr, "machine %s inconnue \n",argv[1]);
		exit(2);
	}
		
		
	
		
	/* Préparation de l'adresse d'attachement */
	adr.sin_family = AF_INET;
	memcpy(&adr.sin_addr.s_addr, hp->h_addr, hp->h_length);
	adr.sin_port = htons(atoi(argv[2]));
	
	
		
	while (1) {
		
		/* Lecture CMD */
		memset(CMD_BRUT, 0, TAILLE);
		memset(CMD_COMPLET, 0, TAILLE);
		memset(NOM_FICHIER, 0, TAILLE);
		
		/* Création de la socket */
		if((socketfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
			perror("Création socket problème ");
			exit(-1);
		}
		
		/* ouverture du service */
		if(connect(socketfd, (struct sockaddr *)&adr,adr_len) == -1){
			perror("connection impossible ");
			exit(-1);
		}
		
		
		
	
		n = read(STDIN_FILENO, CMD_BRUT, TAILLE);
		if(n<0)
			perror("lecture ");
			
		/* TRAITEMENT CHAINE */
		CMD_BRUT[strlen(CMD_BRUT)-1] = 0; //On retire le \n
		CMD = strtok(CMD_BRUT, " ");
		printf("CMD : %s, taille %lu\n",CMD, strlen(CMD));
		NOM_EXTRAIT = strtok(NULL, " ");
		printf("NOM_EXTRAIT : %s\n",NOM_EXTRAIT);
		
		
		
		if(!strcmp(CMD, "LIST")){
			
			/* Ecriture de la commande sur socket */
			n = write(socketfd, CMD, 4*TAILLE);
			if (n < 0) 
				perror("write ");
			
			printf("Passage list\n");
			while((n=read(socketfd,&DATA,4*TAILLE)) != 0){
				printf("%s\n",DATA);
				//n = write(socketfd, DATA, 4*TAILLE);
				if (n < 0) 
					perror("read ");
					
				/* Ne pas oublier de remettre à 0 le buffer */
				memset(DATA, 0, 4*TAILLE);
			}
			
		}else if (!strcmp(CMD, "UPLOAD")){
			
			printf("Passage upload\n");
			
			/* Ecriture de la commande sur socket */
			strcat(CMD_COMPLET, CMD);
			strcat(CMD_COMPLET," ");
			strcat(CMD_COMPLET, NOM_EXTRAIT);
			
			n = write(socketfd, CMD_COMPLET, 4*TAILLE);
			if (n < 0) 
				perror("write ");
			
			/* Ouverture du fichier */
			strcat(NOM_FICHIER, REP_COURANT);
			strcat(NOM_FICHIER, NOM_EXTRAIT);
			printf("-> Ouverture de %s\n",NOM_FICHIER);
			
			//il crée alors dans son répertoire d’exécution un fichier vide portant ce nom.
			if((fichierfd = open(NOM_FICHIER, O_RDONLY, 0666)) == -1){
				perror("open ");
				close(socketfd);
				exit(-1);
			}
			
			//Il lit ensuite les données transmises jusqu’à la fin de la connexion, et les recopie dans le fichier créé précédemment.
			while((n=read(fichierfd,&DATA,4*TAILLE)) != 0){
				//printf("%s",DATA);
				n = write(socketfd, DATA, 4*TAILLE);
				if (n < 0) 
					perror("read ");
					
				/* Ne pas oublier de remettre à 0 le buffer */
				memset(DATA, 0, 4*TAILLE);
			}
			
						
		}else if (!strcmp(CMD, "DOWNLOAD")){
			
			printf("Passage download\n");
			
			/* Ecriture de la commande sur socket */
			strcat(CMD_COMPLET, CMD);
			strcat(CMD_COMPLET," ");
			strcat(CMD_COMPLET, NOM_EXTRAIT);
			
			n = write(socketfd, CMD_COMPLET, 4*TAILLE);
			if (n < 0) 
				perror("write");
				
			strcat(NOM_FICHIER, REP_COURANT);
			strcat(NOM_FICHIER, NOM_EXTRAIT);
			
			//il crée alors dans son répertoire d’exécution un fichier vide portant ce nom.
			if((fichierfd = open(NOM_FICHIER, O_CREAT|O_WRONLY, 0666)) == -1){
				perror("open ");
				close(socketfd);
				exit(-1);
			}
			
			//Il lit ensuite les données transmises jusqu’à la fin de la connexion, et les recopie dans le fichier créé précédemment.
			while(1){
				n = read(socketfd, &DATA,4*TAILLE);
				printf("Impression data %d\n",n);
				if (n < 0)
					perror("read ");
				if(n == 0)
					break;
				if(write(fichierfd, DATA, 4*TAILLE)==-1)
					perror("write ");
				/* Ne pas oublier d'effacer le buffer */
				memset(DATA, 0, 4*TAILLE);
			}			
		}
		printf("FIN \n");
	}
	
	
		
	
	
}