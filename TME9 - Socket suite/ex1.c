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

#define TAILLE 256
struct sigaction action;
pthread_t pidThread;
struct sockaddr_in adr;
socklen_t adr_len = sizeof(struct sockaddr_in);
int connexion, n;
int *connexionToThread;
int socketfd, fichierfd;
char NOM_REPERTOIRE[TAILLE];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void funcTraitement(void *arg){
	
	pthread_mutex_lock(&lock);
	int connect = (*(int*)arg);
	pthread_mutex_unlock(&lock);
	printf("connect : %d\n",connect);
	free(arg);
	char CMD_BRUT[TAILLE];
	char *CMD;
	char *NOM;
	char DATA[4*TAILLE];
	char DATALIST[4*TAILLE];
	char NOM_FICHIER[TAILLE];
	int tube[2];
	
	memset(CMD_BRUT,0,TAILLE);
			
	//Lorsqu’une connexion s’ouvre, il lit la première ligne envoyée et considère que c’est le nom d’un fichier
	n = read(connect,&CMD_BRUT,TAILLE);
	if (n < 0)
		perror("thread read ");
		
	CMD = strtok(CMD_BRUT, " ");
	printf("CMD : %s\n",CMD);
	NOM = strtok(NULL, " ");
	printf("NOM : %s\n",NOM);
	
	if(!strcmp(CMD,"LIST")){
		
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
		close(tube[1]);
		wait(NULL); //Attente du fils
		read(tube[0], DATALIST, 4*TAILLE);
		
		//On envoi la réponse au client maintenant
		n = write(connect, DATALIST, 4*TAILLE);
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
			n = read(connect, &DATA,4*TAILLE);
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

			n = write(connect, DATA, 4*TAILLE);
			if (n < 0) 
				perror("write");
				
			/* Ne pas oublier de remettre à 0 le buffer */
			memset(DATA, 0, 4*TAILLE);
		}
		close(fichierfd);
		
	}
	close(connect);
	pthread_exit((void*)0);

}

void traitSignal(int sig){
	if(SIGSEGV == sig)
		printf("Signal fault reçu\n");
	else
		printf("Signal int ou autre reçu\n");
		
	perror("ERROR");
	close(socketfd);
	close(connexion);
}


int main(int argc, char *argv[]) {
	
	
	int MAXCLIENT = 10;
	
	if(argc <3){
		fprintf(stderr, "usage : %s <port> <repertory>",argv[0]);
		exit(-1);
	}
		
	/* Création de la socket */
	if((socketfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("Création socket problème ");
		exit(-1);
	}
		
	/* Préparation de l'adresse d'attachement */
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = htonl(INADDR_ANY);
	adr.sin_port = htons(atoi(argv[1]));
	
	action.sa_handler = traitSignal;
	//sigaction(SIGCHLD, &action, NULL);
	//sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	
	printf("Configuration signaux : OK\n");
	
	/* Attachement de la socket */
	if (bind(socketfd, (struct sockaddr *)&adr,adr_len)==-1){
		perror("Attachement impossible ");
		exit(-1);
	}
	
	/* ouverture du service */
	if(listen(socketfd, MAXCLIENT) == -1){
		perror("listen impossible ");
		exit(-1);
	}
	
	strcpy(NOM_REPERTOIRE, argv[2]);
	printf("NOM REPERTOIRE : %s\n",NOM_REPERTOIRE);
	/* Boucle de traitement */
	while(1){
		printf("socketfd: %d\n",socketfd);
		connexion = accept(socketfd, (struct sockaddr *)&adr, &adr_len);
		if (connexion == -1) {
			if(errno == EINTR) continue; //Intéruption d'appel, on continue
			else {	//Trop grave, on quitte.
				perror("accept");
				exit(-1);
			}
		}
		connexionToThread = malloc(sizeof(int));
		*connexionToThread = connexion;	//Pointeur vers variable i;
		printf("ConnexionTOThread : %d, connexion %d\n",*connexionToThread,connexion);
		pthread_create(&pidThread,NULL,(void*)funcTraitement,connexionToThread);
		printf("Lancement du Thread :%d\n",(int)pidThread);
		
		//close(connexion);
		
	}
		
	
	
}