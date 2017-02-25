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
	char NOM_REPERTOIRE[TAILLE];
	char DATA[4*TAILLE];
	char DATALIST[4*TAILLE];
	char *CMD;
	char *NOM;
	char CMD_BRUT[TAILLE];
	int tube[2];
	if(argc <3){
		fprintf(stderr, "usage : %s <port> <repertory>",argv[0]);
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
	
	strcpy(NOM_REPERTOIRE, argv[2]);
	printf("%s\n",NOM_REPERTOIRE);
	bzero(&NOM_FICHIER, TAILLE);
	/* Boucle de traitement */
	while(1){
		connexion = accept(socketfd, (struct sockaddr *)&adr, &adr_len);
		
		memset(CMD_BRUT,0,TAILLE);
		
		if (connexion == -1) {
			if(errno == EINTR) continue; //Intéruption d'appel, on continue
			else {	//Trop grave, on quitte.
				perror("accept");
				exit(-1);
			}
		}
		
		if(fork() == 0){
			
			//Lorsqu’une connexion s’ouvre, il lit la première ligne envoyée et considère que c’est le nom d’un fichier
			n = read(connexion,&CMD_BRUT,TAILLE);
			if (n < 0)
				perror("read ");
				
			CMD = strtok(CMD_BRUT, " ");
			printf("CMD : %s\n",CMD);
			NOM = strtok(NULL, " ");
			printf("NOM : %s\n",NOM);
				
			if(!strcmp(CMD,"LIST")){
				printf("PASSAGE\n");
				pipe(tube); //Création d'un tube pour communication (récupération du résultat de ls);
				if(fork() == 0){
					dup2(tube[1], STDOUT_FILENO);
					dup2(tube[1], STDERR_FILENO); //On place également l'erreur dans le flux de sortie;
					close(tube[0]);
					close(tube[1]);
					execlp("ls", "ls","-l", NOM_REPERTOIRE,NULL);
					perror("execlp");
					exit(2);
					
				}
				close(1);
				wait(NULL); //Attente du fils
				read(tube[0], DATALIST, 4*TAILLE);
				
				//On envoi la réponse au client maintenant
				n = write(connexion, DATALIST, 4*TAILLE);
				if (n < 0) 
					perror("write ");
				
				
			}else if(!strcmp(CMD,"UPLOAD")){
				
				strcat(NOM_FICHIER, NOM_REPERTOIRE);
				strcat(NOM_FICHIER, NOM);
				
				//NOM_FICHIER[strlen(NOM_FICHIER)-1] = '2';
				
				printf("NOM FINAL : %s",NOM_FICHIER);
				
				
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
						perror("read ");
					if(n == 0)
						break;
					if(write(fichierfd, DATA, 4*TAILLE)==-1)
						perror("write ");
					/* Ne pas oublier d'effacer le buffer */
					memset(DATA, 0, 4*TAILLE);
				}
				
				printf("Fin d'écriture du fichier \n");
				
			}else if(!strcmp(CMD,"DOWNLOAD")){
				
				strcat(NOM_FICHIER, NOM_REPERTOIRE);
				strcat(NOM_FICHIER, NOM);
				/*Ouverture du fichier */
				fichierfd = open(NOM_FICHIER, O_RDONLY, 0666);
				
				/* Boucle de traitement */
				while((n=read(fichierfd,&DATA,4*TAILLE)) != 0){

					n = write(connexion, DATA, 4*TAILLE);
					if (n < 0) 
						perror("write");
						
					/* Ne pas oublier de remettre à 0 le buffer */
					memset(DATA, 0, 4*TAILLE);
				}
				close(fichierfd);
				
			}
			close(connexion);
			exit(1);
			
		}
		close(connexion);
		
	}
		
	
	
}