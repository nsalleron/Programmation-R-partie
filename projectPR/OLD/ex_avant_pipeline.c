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
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define TAILLE 1024*2
struct sigaction action;
pthread_t pidThread;
int pidFils;
struct sockaddr_in adr;
socklen_t adr_len = sizeof(struct sockaddr_in);
int connexion, n;
int *connexionToThread;
int socketfd, mimefd, logfd;
char NOM_REPERTOIRE[TAILLE];
int MAXCLIENT = 10;
char CWD[TAILLE];
char *STD_REP = "HTTP/1.1 ";

pthread_mutex_t mutexConnexion = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexLogFile = PTHREAD_MUTEX_INITIALIZER;

int nbTotalConnexion = 0;

struct infoRequete{
	int connectfd;
	struct in_addr ipAddr;
	pid_t pid;
	time_t curtime;
	pthread_t pthreadid;
	char *line_request;
	char *ret_code;
	int total_size_send;
};

struct infoRequete *info;


char *getMimeInfo(char *PATH_FILE);
void recordToLogFile(struct infoRequete *infoRequete);
void traitSigAlarm(int sig);




void funcTraitement(void *arg){
	
	/* Récupération des informations */
	struct infoRequete info;
	info = (*(struct infoRequete*)arg);
	info.pthreadid = pthread_self();
	info.total_size_send = 0;
	int connect = info.connectfd;
	free(arg);
	
	printf("%lu | DEBUT TRAITEMENT de %d\n",(long)pthread_self(),connect);
	
	/*Descripteur de fichier */
	int fichierfd;
	
	/* Pipe du thread */
	int localPipe[2];
	int filsPipe[2];
	
	/*Integer erreur */
	int bErreur = 0;
	int bRepertory = 0;
	int bExecutable = 0;
	
	/* Commande brut récupéré par le serveur */
	char CMD_BRUT[TAILLE];
	
	/* Commande après strtok */
	char *CMD;
	char *FILE;
	char *HTTPVER;
	
	/* fichier requis */
	char PATH_FILE[TAILLE];
	
	/* Vérification du fichier */
	struct stat statFile;
	
	/* ENOENT si fichier inacessible */
	/* EACCES si droits insuffisant */
	int Errno; 
	
	/* Les données de la réponse */
	char DATA[2*TAILLE];
	char DATA_BODY[2*TAILLE];
	char DATA_FILE[2*TAILLE];
	
	/* Initialisation des variables */
	memset(CMD_BRUT,0,TAILLE);
	memset(DATA,0,2*TAILLE);
	
	/* strtok_r */
	char *saveptr1;
	
	//Lecture de la requête
	n = read(connect,&CMD_BRUT,TAILLE);
	if (n < 0)
		perror("thread read ");
	
	CMD = strtok_r(CMD_BRUT, " ",&saveptr1);
	printf("%lu | CMD : %s, taille %lu\n",(long)pthread_self(),CMD, strlen(CMD));
	FILE = strtok_r(NULL, " ",&saveptr1);
	printf("%lu | NOM : %s, taille %lu\n",(long)pthread_self(),FILE,strlen(FILE));
	HTTPVER = strtok_r(NULL, "\r",&saveptr1);
	printf("%lu | HTTPVER : %s, taille %lu\n",(long)pthread_self(),HTTPVER,strlen(HTTPVER));
	
	/* Mise à jour des informations */
	info.line_request = malloc((strlen(CMD)+strlen(FILE)+strlen(HTTPVER)+1)*sizeof(char));
	info.ret_code = malloc(sizeof(char)*4);
	strcat(info.line_request, CMD);
	info.line_request[strlen(CMD)] = ' ';
	strcat(info.line_request, FILE);
	info.line_request[strlen(CMD)+strlen(FILE)+1] = ' ';
	strcat(info.line_request, HTTPVER);
	
	/* Construction du chemin avec le répertoire de travail */
	strcat(PATH_FILE, CWD);
	strcat(PATH_FILE, FILE);
	
	/* GET FILE */
	if(!strcmp(CMD,"GET")){
		
				
		/* Le fichier est accessible */
		if((fichierfd = open(PATH_FILE, O_RDONLY, S_IRUSR|S_IRGRP|S_IROTH)) == -1){
			perror("open ");
			bErreur = 1;
			Errno = errno;
		}
		
		if(!bErreur){
			
			/* Vérification du fichier */
			if (stat(PATH_FILE,&statFile) == -1){
				perror("Problème dans le fstat\n");
			}
			/* Répertoire */
			if(S_ISDIR(statFile.st_mode)){
				printf("It's a repertory\n");
				bRepertory = 1;
			}
			
			if(!bRepertory){
	
				if(!access(PATH_FILE, X_OK)){
					bExecutable = 1;
				}
				printf("Valeur bExecutable : %d du fichier %s\n",bExecutable,PATH_FILE);
				
				if(!bExecutable){
					/* Construction de la réponse */
					strcat(DATA, STD_REP);
					strcat(DATA, "200 OK\r\n");
					info.ret_code = "200";
					printf("retcode : %s\n",info.ret_code);
					
					/* Content type */
					strcat(DATA, "Content-Type: ");
					char *strtType = getMimeInfo(PATH_FILE);
					strcat(DATA, strtType);
					strcat(DATA, "\r\n"); /* Caractère de fermeture */
					free(strtType);	//On libère les caractères du TAS
					
					/*Calcul de la taille du fichier */
					char taille[150];
					int tailleTot = 0;
					while(1){
						n = read(fichierfd, DATA_FILE,TAILLE);
						
						//printf("%lu | Impression data %d sur socket %d\n",(long)pthread_self(),n,connect);
						if (n < 0)
							perror("read ");
							
						/*Ajout de la taille total */
						tailleTot +=n;
						
						if(n == 0)
							break;
							
					}
					
					/* Content length */
					printf("%d\n",tailleTot);
					sprintf(taille,"%d",tailleTot);
					strcat(DATA, "Content-Length: " );
					strcat(DATA, taille);
					strcat(DATA, "\r\n\n");
					
					/*Ecriture de l'entête */
					n = write(connect, DATA, strlen(DATA));
					if (n < 0)
						perror("write entete");
					
					/* Impression du contenu */
					lseek(fichierfd, 0, SEEK_SET);
					while(1){
						n = read(fichierfd, DATA_FILE,TAILLE);
						
						printf("%lu | Lecture de data %d sur fichier %d\n",(long)pthread_self(),n,connect);
						if (n < 0)
							perror("read ");
							
						n = write(connect, DATA_FILE, n);
						printf("%lu | Ecritre de data %d sur socket  %d\n",(long)pthread_self(),n,connect);
						if (n < 0)
							perror("write ");
						
						if(n == 0)
							break;
									
						/* Ne pas oublier d'effacer le buffer */
						memset(DATA_FILE, 0, TAILLE);
					}
					
					printf("DATA : \n\n%s",DATA);
					printf("FIN DATA\n");
					
				}else{
					printf("%d | exécution \n",getpid());
					int returnValue = 0;
					pipe(localPipe); //Création d'un tube pour communication (récupération du résultat du fils);
					
					//Premier fils handler de l'écriture et du signal
					if((pidFils = fork()) == -1){
						perror("fork ");
						exit(-1);
					}
					
					/*Lancement du premier fils */
					if(pidFils == 0){
						printf("%d | exécution \n",getpid());
						pipe(filsPipe);
						signal(SIGALRM, traitSigAlarm);
						
						//Second fils, récupération de la valeur du retour et attente des 10 secondes
						if((pidFils = fork()) == -1){
							perror("fork ");
							exit(-1);
						}
						/*petit fils ici */
						if(pidFils == 0){
							printf("%d | exécution finale du fichier : %s \n",getpid(),PATH_FILE);
							dup2(filsPipe[1], STDOUT_FILENO);
							dup2(filsPipe[1], STDERR_FILENO);
							close(filsPipe[0]);
							close(filsPipe[1]);
							execl(PATH_FILE, PATH_FILE, NULL);
							perror("execlp");
							exit(-1);	
						}else{
							dup2(localPipe[1], STDOUT_FILENO);
							dup2(localPipe[1], STDERR_FILENO);
							close(filsPipe[1]);
							alarm(10); //Mise en place de l'alarm;
							int valEcrite = 0;
							/* Attente du fils */
							waitpid(pidFils, &returnValue, 0);
							
							/* Selon la valeur de retour */
							if(returnValue==0){
								
								/* Construction de la réponse */
								strcat(DATA, STD_REP);
								strcat(DATA, "200 OK\r\nContent-Type: ");
								
								info.ret_code = "200";
								
								/* A RECHERCHER si pas mieux */
								strcat(DATA, "text/plain");
								strcat(DATA, "\r\n\n"); /* Caractère de fermeture */
								
								while(1){
									n = read(filsPipe[0], DATA_FILE,TAILLE);
									//printf("%lu | Impression data %d sur socket %d\n",(long)getpid(),n,connect);
									if (n < 0)
										perror("read ");
									if(n == 0)
										break;
									strcat(DATA, DATA_FILE);
									/* Ne pas oublier d'effacer le buffer */
									memset(DATA_FILE, 0, TAILLE);
								}
								/*Ecriture du résultat de la commande */
								valEcrite = write(connect, DATA, strlen(DATA));
								close(connect);
								
								/* Technique pour récupérer le code de retour à 3 chiffres */
								write(STDOUT_FILENO, "200" , sizeof("200"));
							}else {
								/* Technique pour récupérer le code de retour à 3 chiffres */
								write(STDOUT_FILENO, "500" , sizeof("500"));
							}	
							
							write(STDOUT_FILENO, &valEcrite, sizeof(int));
							
							exit(returnValue);
						} /* <- Fin des petits fils */
					/*Fin premier fils, ici thread */
					}else{	/* C'est lui qui doit écrire sur le socket */
						printf("%d | exécution thread originel\n",getpid());
						
						/* Technique pour récupérer le code de retour à 3 chiffres */
						char *ret_code;
						int *val_Ecrite;
						ret_code = malloc(sizeof(char)*3);
						val_Ecrite = malloc(sizeof(int));
						
						close(localPipe[1]);
						waitpid(pidFils, &returnValue, 0);
						read(localPipe[0], ret_code, sizeof(ret_code));
						printf("ret_code : %s\n",ret_code);
						
						if(returnValue==0 && !strcmp(ret_code, "200")){
							int val = 0;
							read(localPipe[0], &val_Ecrite,sizeof(int)); /* Nombre d'octet écrit dans le fils*/
							info.ret_code = "200";
							info.total_size_send += *val_Ecrite;
						}else{ /*Dans le cas d'un echec de l'exécution */
							read(localPipe[0], &val_Ecrite,sizeof(int)); /* Nombre d'octet écrit : ici 0 */
							strcat(DATA, ret_code);
							strcat(DATA, " Internal Server Error\r\n\n"); 
							info.ret_code = "505";
							info.total_size_send +=write(connect, DATA, strlen(DATA));
						}
						
					} /* <- Fin else socket traitement */
				}/* <- Fin cas exécutable */
			}else if (bRepertory){
				//Ecriture des infos du répertoire
				printf("A FAIRE // Répertoire\n");
				
			}
						
		}else {		/* Traitement de l'erreur */
			printf("%lu | Traitement code erreur : %d\n",(long)pthread_self(),Errno);
			strcat(DATA, STD_REP);
			
			switch (Errno) {
				case ENOENT: strcat(DATA, "404 Not Found\r\n\n"); info.ret_code = "404"; break;
				case EACCES: strcat(DATA, "403 Forbidden\r\n\n"); info.ret_code = "403" ;break;
				default: strcat(DATA, "418 	I’m a teapot\r\n\n"); info.ret_code = "418";
				
			}
			info.total_size_send += write(connect, DATA, strlen(DATA));
		}
		
		/*Enregistrement des informations dans le fichier de log */
		recordToLogFile(&info);
		
		/*Fermeture propre du socket */
		if (shutdown(info.connectfd, SHUT_RDWR) < 0) // secondly, terminate the 'reliable' delivery
			if (errno != ENOTCONN && errno != EINVAL) // SGI causes EINVAL
				perror("shutdown");
		if (close(info.connectfd) < 0) // finally call close()
			perror("close");
		
		pthread_mutex_lock(&mutexConnexion);
		nbTotalConnexion--;
		printf("NOMBRE TOTOAL DE CONNEXION ACTUALISE PAR THREAD :%d\n",nbTotalConnexion);
		pthread_mutex_unlock(&mutexConnexion);
		
		printf("%lu | FIN TRAITEMENT de %d \n",(long)pthread_self(),connect);
		pthread_exit((void*)0);
	}
}
/*
 *	Ecriture vers le fichier de log
 *
 */
void recordToLogFile(struct infoRequete *info){
	
	struct tm *loctime;
	char date[50];
	char record[150];
	char ip[INET_ADDRSTRLEN];
	int n;
	
	/* Représentation en str */
	inet_ntop( AF_INET, &info->ipAddr, ip, INET_ADDRSTRLEN );
	
	/* Représentation locale du temps. */
	loctime = localtime(&info->curtime);
	
	/* Ecriture de la date dans un format standard */
	sprintf(date,"%s",asctime(loctime));
	date[24] = 0;	/*D'après le man, 26 caractère au total, on supprime ici le caractère \n pour le log. */
		
	//printf("%s, taille %lu\n",date,strlen(date));
	
	sprintf(record,"%s %s %d %lu %s %s %d\n",ip
		,date
		,info->pid
		,(unsigned long)info->pthreadid
		,info->line_request
		,info->ret_code
		,info->total_size_send);
		
	pthread_mutex_lock(&mutexLogFile);
	if((n = write(logfd, record, strlen(record))) == -1){
		pthread_mutex_unlock(&mutexLogFile);
		perror("write record");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock(&mutexLogFile);
	
}


/*
 *	Recherche de l'extension mime
 *
 */
char *getMimeInfo(char *PATH_FILE){
	/*Extraction de l'extension */
	/* A REFAIRE AVEC regexec */
	
	const char *str_request = strrchr(PATH_FILE, '.');
	memmove((void*)str_request, str_request+1, strlen(str_request));
	
	/* On recherche l'expression régulière */
	lseek(mimefd, 0, SEEK_SET);
	FILE *file ;
	int bStrOk = 1;
	char line[256];
	char *strtType;
	char *strtContent;
	char *strtResult;
	char *saveptr1;
	
	if((file = fdopen(mimefd, "r")) == NULL){
		perror("fdopen");
		pthread_exit((void *)0);
	} 

	while (fgets(line, sizeof(line), file)) {
		
		/* On est à la bonne ligne du fichier */
		if(strstr(line, str_request) != NULL && line[0] != '#') {
			/* On conserve le premier type */
			strtType = strtok_r(line, "\t",&saveptr1);
			
			strtContent = strtok_r(NULL, "\t",&saveptr1);
		
			while (bStrOk) {
				
				/* On sort de la boucle si c'est le bon caractère */
				if(strtContent == NULL)
					break;
			
				strtResult = strtok_r(strtContent, " ",&saveptr1);
				while (bStrOk) {
					if(strtResult == NULL)
						break;
					if(!strcmp(strtResult, str_request))
						bStrOk = 0;
					strtResult = strtok_r(NULL, " ",&saveptr1);
				}
				if(bStrOk != 0)
					strtContent = strtok_r(NULL, "\t",&saveptr1);
			}
			
			/* A partir d'ici nous sommes dans la bonne ligne */
			if(strtResult != NULL && bStrOk == 0){
				break;
			}
			
		}
		memset(line, 0, strlen(line));
	}
	char *ret;
	ret = malloc(strlen(strtType)*sizeof(char));
	memcpy(ret, strtType, strlen(strtType));
	printf("Valeur avant retour de ret : %s\n",ret);
	
	return ret;
}
/*
 *	traitement du signal alarm;
 *
 */
void traitSigAlarm(int sig){
	kill(pidFils, SIGINT);
}

/*
 *	Fermeture normale du serveur;
 *
 */
void traitSignal(int sig){
	if(SIGSEGV == sig)
		printf("\nSignal fault reçu\n");
	else
		printf("\nSignal int ou autre reçu\n");
	
	if (shutdown(socketfd, SHUT_RDWR) < 0) // secondly, terminate the 'reliable' delivery
		if (errno != ENOTCONN && errno != EINVAL) // SGI causes EINVAL
			perror("shutdown");
	if (close(socketfd) < 0) // finally call close()
		perror("close");
	exit(0);
}


int main(int argc, char *argv[]) {
	
	if(argc <3){
		fprintf(stderr, "usage : %s <port> <client max> <other number>",argv[0]);
		exit(-1);
	}
	/* Ouverture des informations de log */
	//On convient de placer ce fichier dans le répertoire /tmp, avec comme nom votre identifiant à 7 chiffres précédé de "http" et suivi de ".log".
	if((logfd = open("./tmp/http3504018.log", O_CREAT | O_RDWR | O_APPEND, S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR)) == -1){
		perror("mime open");
		exit(-1);
	}
	
	/* Ouverture du fichier mime */
	if((mimefd = open("/etc/mime.types", O_RDONLY, S_IRUSR|S_IRGRP|S_IROTH)) == -1){
		perror("ouverture mime par defaut ");
		/*Utilisation du mime du serveur */
		if((mimefd = open("mime.types", O_RDONLY, S_IRUSR|S_IRGRP|S_IROTH)) == -1){
			perror("mime open");
			exit(-1);
		}
	}
		
	/* Création de la socket */
	if((socketfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("Création socket problème ");
		exit(-1);
	}
	
	/*Récupération du répertoire de travail */
	getcwd(CWD, sizeof(CWD));
	printf("Répertoire courant : %s\n", CWD);
	
	/* Récupération du nombre max de client */
	MAXCLIENT = atoi(argv[2]);
	printf("Nombre total de connexion :%d\n",MAXCLIENT);
		
	/* Préparation de l'adresse d'attachement */
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = htonl(INADDR_ANY);
	adr.sin_port = htons(atoi(argv[1]));
	
	action.sa_handler = traitSignal;
	sigaction(SIGINT, &action, NULL);
	printf("Configuration signaux : OK\n");
	printf("PID du serveur principal : %d\n", getpid());
	/* Attachement de la socket */
	if (bind(socketfd, (struct sockaddr *)&adr,adr_len)==-1){
		perror("Attachement impossible ");
		exit(-1);
	}
	
	/* ouverture du service */
	if(listen(socketfd, 0) == -1){
		perror("listen impossible ");
		exit(-1);
	}
	
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
		pthread_mutex_lock(&mutexConnexion);
		if(nbTotalConnexion < MAXCLIENT){
			nbTotalConnexion++;
			printf("NOMBRE TOTAL DE CONNEXION ACTUALISE : %d\n", nbTotalConnexion);
			printf("Numéro du descripteur : %d\n",connexion);
			pthread_mutex_unlock(&mutexConnexion);
			
			/* Informations pour traitement et journalisation */
			info = malloc(sizeof(struct infoRequete));
			info->connectfd = connexion;
			struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&adr;
			info->ipAddr = pV4Addr->sin_addr;
			info->curtime = time (NULL);
			info->pid = getpid();
			
			printf("LANCEMENT %d\n",info->connectfd);
			
			pthread_create(&pidThread,NULL,(void*)funcTraitement,info);
		}else{
			//Message de rejet
			printf("PASSAGE ELSE\n");
			close(connexion);
		}
		pthread_mutex_unlock(&mutexConnexion);
	}
}