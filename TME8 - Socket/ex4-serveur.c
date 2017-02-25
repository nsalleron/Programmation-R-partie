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

#define TAILLE 256
struct sigaction action;


/* Pour la capture du signal sigchild */
void finFils(int sig){
	printf("Fin d'un fils\n");
	wait(NULL);
}

int main(int argc, char *argv[]) {
	struct sockaddr_in adr;
	socklen_t adr_len = sizeof(struct sockaddr_in);
	int connexion, n;
	int socketfd, fichierfd;
	char NOM_FICHIER[TAILLE];
	char DATA[4*TAILLE];
	if(argc <2){
		fprintf(stderr, "usage : %s <port>",argv[0]);
		exit(-1);
	}
		
	action.sa_handler = finFils;
	
	sigaction(SIGCHLD, &action, NULL);
		
	/* Création de la socket */
	if((socketfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("Création socket problème ");
		exit(-1);
	}
		
	/* Préparation de l'adresse d'attachement */
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = htonl(INADDR_ANY);
	adr.sin_port = htons(atoi(argv[1]));
	
	/* Attachement de la socket */
	if (bind(socketfd, (struct sockaddr *)&adr,adr_len)==-1){
		perror("Attachement impossible ");
		exit(-1);
	}
	
	/* ouverture du service */
	if(listen(socketfd, 10) == -1){
		perror("listen impossible ");
		exit(-1);
	}
	
	bzero(&NOM_FICHIER, TAILLE);
	/* Boucle de traitement */
	while(1){
		connexion = accept(socketfd, (struct sockaddr *)&adr, &adr_len);
		if (connexion == -1) {
			if(errno == EINTR) continue; //Intéruption d'appel, on continue
			else {	//Trop grave, on quitte.
				perror("accept");
				exit(-1);
			}
		}
		
		if(fork() == 0){
			//Lorsqu’une connexion s’ouvre, il lit la première ligne envoyée et considère que c’est le nom d’un fichier
			memset(NOM_FICHIER, 0, TAILLE);
			n = read(connexion,&NOM_FICHIER,TAILLE);
			//n = recvfrom(connexion, &NOM_FICHIER, TAILLE , 0, (struct sockaddr *) &adr, &adr_len);
			if (n < 0)
				perror("ERROR in recvfrom");
				
			
			
			
			/* Modification du nom du fichier volontaire car même répertoire */
			printf("%s\n",NOM_FICHIER);
			
			if(1)
				return 0;
			
			NOM_FICHIER[strlen(NOM_FICHIER)+1] = NOM_FICHIER[strlen(NOM_FICHIER)];
			NOM_FICHIER[strlen(NOM_FICHIER)] = '2';
			printf("Nom fichier : %s\n",NOM_FICHIER);
			
			//il crée alors dans son répertoire d’exécution un fichier vide portant ce nom.
			if((fichierfd = open(NOM_FICHIER, O_CREAT|O_WRONLY, 0666)) == -1){
				perror("open ");
				close(connexion);
				exit(-1);
			}
			
			//Il lit ensuite les données transmises jusqu’à la fin de la connexion, et les recopie dans le fichier créé précédemment.
			while(1){
				n = read(connexion, &DATA,4*TAILLE);
				printf("Impression data %d\n",n);
				if (n < 0)
					perror("ERROR in recvfrom");
				if(n == 0)
					break;
				if(write(fichierfd, DATA, 4*TAILLE)==-1)
					perror("write ");
				/* Ne pas oublier d'effacer le buffer */
				memset(DATA, 0, 4*TAILLE);
			}
			
			printf("Fin d'écriture du fichier \n");
			exit(1);
			
		}
		close(connexion);
		
	}
		
	
	
}