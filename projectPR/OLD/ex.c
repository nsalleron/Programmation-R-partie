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
pthread_mutex_t mutexFile = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSocket = PTHREAD_MUTEX_INITIALIZER;

int nbTotalConnexion = 0;

struct infoRequete{
	int connectfd;
	struct in_addr ipAddr;
	int connexionId;
	pid_t pid;
	time_t curtime;
	pthread_t pthreadid;
	char *line_request;
	char *ret_code;
	int total_size_send;
};

struct repThread{
	int connectfd;
	struct infoRequete infoReq;
	int id;
	pthread_t idThreadParent;
	int* nbThreadActif;
	char *CMD;
	char *FILE;
	char *HTTPVER;
	char *PATH_FILE;
};

struct infoRequete *info;
pthread_t *pthreadAlive;
int bThreadActif = 1;



char *getMimeInfo(char *PATH_FILE);
void recordToLogFile(struct infoRequete *infoRequete);
void traitSigAlarm(int sig);
void funcRep(void *arg);

void afficherConnexions(){
	int i;
	/*Affichage des connexions en cours */
	printf(" ------------------------------\n");
	printf("|     Connexions en cours      |\n");
	printf(" ------------------------------\n");
	for(i = 0;i<MAXCLIENT;i++)
		if(pthreadAlive[i] != 0 && i<10)
			printf("| Thread : %d = %lu  |\n",i,(long)pthreadAlive[i]);
		else if(pthreadAlive[i] != 0 && i<100)
			printf("| Thread : %d = %lu |\n",i,(long)pthreadAlive[i]);
		else if(pthreadAlive[i] != 0 && i<1000)
		printf("| Thread : %d = %lu|\n",i,(long)pthreadAlive[i]);
	printf(" ------------------------------\n");
}


void funcTraitement(void *arg){
	
	/* Récupération des informations */
	struct infoRequete info;
	info = (*(struct infoRequete*)arg);
	info.pthreadid = pthread_self();
	info.total_size_send = 0;
	int connect = info.connectfd;
	free(arg);
	
	printf("%d de %lu | DEBUT TRAITEMENT de %d\n",connect,(long)pthread_self(),connect);
	
	/* Commande brut récupéré par le serveur */
	char CMD_BRUT[TAILLE];
	
	/* Commande après strtok */
	char *CMD;
	char *FILE;
	//char *HTTPVER;
	
	/* fichier requis */
	char PATH_FILE[TAILLE];
	
	/* Les données de la réponse */
	char DATA[2*TAILLE];
	
	/* Initialisation des variables */
	memset(CMD_BRUT,0,TAILLE);
	memset(DATA,0,2*TAILLE);
	
	/* strtok_r */
	char *saveptr1;
	
	/* Tableau pour les threads fils */
	int tailleThreadRequest = 0;
	pthread_t *arrayRequestThread = malloc(tailleThreadRequest * sizeof(pthread_t));

	
	/* Thread Rep */
	struct repThread *rep;
	memset(CMD_BRUT, 0, TAILLE);
	
	/* Lecture des requêtes */
	while(((n = read(connect,CMD_BRUT,TAILLE)) !=0) && bThreadActif == 1){
		
		
		if(n<0){
			if(errno == EINTR) continue; //Intéruption d'appel, on continue
			else{
				perror("read thread");
				close(connect);
				pthread_exit((void*)EXIT_FAILURE);
			}
		}
			
		
		CMD = strtok_r(CMD_BRUT, " ",&saveptr1);
		if(CMD == NULL)
			continue;
		printf("%d de %lu | CMD : %s, taille %lu\n",connect,(long)pthread_self(),CMD, strlen(CMD));
		
		FILE = strtok_r(NULL, " ",&saveptr1);
		if(FILE == NULL)
			continue;
		
		printf("%d de %lu | NOM : %s, taille %lu\n",connect,(long)pthread_self(),FILE,strlen(FILE));
		//HTTPVER = strtok_r(NULL, "\r",&saveptr1);
		//printf("%d de %lu | HTTPVER : %s, taille %lu\n",connect,(long)pthread_self(),HTTPVER,strlen(HTTPVER));
		
		/* Mise à jour des informations */
		info.line_request = malloc((strlen(CMD)+strlen(FILE)+1)*sizeof(char));
		info.ret_code = malloc(sizeof(char)*4);
		strcat(info.line_request, CMD);
		info.line_request[strlen(CMD)] = ' ';
		strcat(info.line_request, FILE);
		
		/* Construction du chemin avec le répertoire de travail */
		strcat(PATH_FILE, CWD);
		strcat(PATH_FILE, FILE);
		
		rep = malloc(sizeof(struct repThread));
		
		/* Mise en place des informations */
		rep->connectfd = info.connectfd;
		rep->CMD = malloc(sizeof(char)*(strlen(CMD)+1));
		memset(rep->CMD, 0, strlen(CMD));
		strcpy(rep->CMD, CMD);
		rep->CMD[strlen(rep->CMD)] = 0;

		rep->FILE = malloc(sizeof(char)*(strlen(FILE)+1));
		memset(rep->FILE, 0, strlen(FILE));
		strcpy(rep->FILE,FILE);
		rep->FILE[strlen(rep->FILE)] = 0;
		//printf("File : %s length : %lu\n",rep->FILE,strlen(rep->FILE));
		
		rep->PATH_FILE = malloc(sizeof(char)*(strlen(PATH_FILE)+1));
		memset(rep->PATH_FILE, 0, strlen(PATH_FILE));
		strcpy(rep->PATH_FILE,PATH_FILE);
		rep->PATH_FILE[strlen(rep->PATH_FILE)] = 0;
		
		memcpy(&rep->infoReq, &info, sizeof(info));
		
		
		if(tailleThreadRequest!=0){		/* Placement de l'identifiant pour les "fils" du 0 */
			rep->id = tailleThreadRequest;
			rep->idThreadParent = arrayRequestThread[tailleThreadRequest-1];
		}else{							/* Nous sommes dans le premier thread de réponse, il n'attend personne */
			rep->id = 0;
			rep->idThreadParent = 0;	/* Premier thread */
		}
		
		/* Pour décrémenter par la suite, le nombre de threads */
		rep->nbThreadActif = &tailleThreadRequest;
		
		/* Augmentation de la taille du tableau pour les suivants */
		tailleThreadRequest++;
		printf("%d de %lu | DEBUG | NB THREAD \"FILS\" : %d\n",connect, (long) pthread_self(),*rep->nbThreadActif);
		
		
		pthread_t *reallocArrayRequestThread = realloc(arrayRequestThread, tailleThreadRequest * sizeof(long));
		if (reallocArrayRequestThread) {
		  arrayRequestThread = reallocArrayRequestThread;
		} else {
		  perror("realloc");
		  pthread_exit((void *)EXIT_FAILURE);
		}
		
		/*	Création du thread */
		pthread_create(&pidThread,NULL,(void*)funcRep,rep);
		
		/*Enregistrement du nouveau thread parent */
		arrayRequestThread[tailleThreadRequest-1] = pidThread;
		printf("%d de %lu | DEBUG | NOUVEAU THREAD ID : %d | TID : %lu\n",connect, (long) pthread_self(),tailleThreadRequest-1, (long) arrayRequestThread[tailleThreadRequest-1]);
		
		
		memset(PATH_FILE, 0, TAILLE);
		memset(CMD_BRUT, 0, TAILLE);
		
		//pthread_join(pidThread, NULL);
	}
	/* Quand read retourne 0 c'est que la connexion TCP à été fermé de l'autre côté */
	/* Décrément du nombre total de connexion */
	printf("%d de %lu | FERMETURE CONNEXION\n",connect,(long)pthread_self());
	
	pthread_mutex_lock(&mutexConnexion);
	nbTotalConnexion--;
	pthreadAlive[info.connexionId] = 0;
	pthread_mutex_unlock(&mutexConnexion);
	
	/* Fermeture propre du socket */
	if (shutdown(connect, SHUT_RDWR) < 0) // secondly, terminate the 'reliable' delivery
		if (errno != ENOTCONN && errno != EINVAL) // SGI causes EINVAL
			perror("shutdown");
	if (close(connect) < 0) // finally call close()
		perror("close");
	
	pthread_mutex_lock(&mutexConnexion);
	afficherConnexions();
	pthread_mutex_unlock(&mutexConnexion);
	
	pthread_exit((void*)EXIT_SUCCESS);
}
	
	
void funcRep(void *arg){
	
	/* Sauvegarde de l'adresse pour décrémenter le nombre de thread */
	int *nbThreadActif;	
	nbThreadActif = ((struct repThread *)arg)->nbThreadActif;
	
	/* Récupération des informations */
	struct repThread info;
	info = (*(struct repThread*)arg);
	info.infoReq.pthreadid = pthread_self();
	info.infoReq.total_size_send = 0;
	int connect = info.connectfd;
	free(arg);
	fflush(NULL);
	
	/* Affichage des informations */
	printf("%d de %lu | THREADREP NUMERO %d\n",connect,(long)pthread_self(),info.id);
	printf("%d de %lu | THREADREP INCREMENT %d\n",connect,(long)pthread_self(),*nbThreadActif);
	printf("%d de %lu | THREADREP PARENT %lu\n",connect,(long)pthread_self(),(long)info.idThreadParent);
	printf("%d de %lu | THREADREP DEBUT TRAITEMENT de %s\n",connect,(long)info.infoReq.pthreadid,info.FILE);
	
	/*Descripteur de fichier */
	int fichierfd;
	
	/* Pipe du thread */
	int localPipe[2];
	int filsPipe[2];
	
	/*Integer erreur */
	int bErreur = 0;
	int bRepertory = 0;
	int bExecutable = 0;
	
	/* Vérification du fichier */
	struct stat statFile;
	
	/* ENOENT si fichier inacessible */
	/* EACCES si droits insuffisant */
	int Errno; 
	
	/* Les données de la réponse */
	char DATA[2*TAILLE];
	char DATA_FILE[2*TAILLE];
	
	/* Initialisation des variables */
	memset(DATA,0,2*TAILLE);
	
	
	
	/* GET FILE */
	if(!strcmp(info.CMD,"GET")){
		
		//printf("info PATH_FILE: %s\n",info.PATH_FILE);
		/* Le fichier est accessible */
		if((fichierfd = open(info.PATH_FILE, O_RDONLY, S_IRUSR|S_IRGRP|S_IROTH)) == -1){
			printf("%lu | perror open ", (long)info.infoReq.pthreadid );
			perror(" open ");
			bErreur = 1;
			Errno = errno;
		}
		
		if(!bErreur){
			
			/* Vérification du fichier */
			if (stat(info.PATH_FILE,&statFile) == -1){
				perror("Problème dans le fstat\n");
			}
			/* Répertoire */
			if(S_ISDIR(statFile.st_mode)){
				printf("It's a repertory\n");
				bRepertory = 1;
			}
			
			if(!bRepertory){
	
				if(!access(info.PATH_FILE, X_OK)){
					bExecutable = 1;
				}
				//printf("Valeur bExecutable : %d du fichier %s\n",bExecutable,info.PATH_FILE);
				
				if(!bExecutable){
					
					
					/* Construction de la réponse */
					strcat(DATA, STD_REP);
					strcat(DATA, "200 OK\r\n");
					info.infoReq.ret_code = "200";
					//printf("retcode : %s\n",info.infoReq.ret_code);
					
					/* Content type */
					strcat(DATA, "Content-Type: ");
					pthread_mutex_lock(&mutexFile);
					char *strtType = getMimeInfo(info.PATH_FILE);
					pthread_mutex_unlock(&mutexFile);
					strcat(DATA, strtType);
					
					strcat(DATA, "\r\n"); /* Caractère de fermeture */
					free(strtType);	//On libère les caractères du TAS
					
					/*Calcul de la taille du fichier */
					char taille[150];
					int tailleTot = 0;
					pthread_mutex_lock(&mutexFile);
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
					pthread_mutex_unlock(&mutexFile);
					
					/* Content length */
					//printf("%d\n",tailleTot);
					sprintf(taille,"%d",tailleTot);
					strcat(DATA, "Content-Length: " );
					strcat(DATA, taille);
					strcat(DATA, "\r\n\n");
					
					/*Ecriture de l'entête */
					n = write(connect, DATA, strlen(DATA));
					if (n < 0)
						perror("write entete");
						
					/* Attente du père avant d'écrire sur la socket */
					if(info.id != 0){
						printf("%d de %lu | DEBUG | THREAD ID %d ATTENTE DE %lu\n",connect, (long) pthread_self(),info.id,(long)info.idThreadParent);
						pthread_join(info.idThreadParent, (void*)0);
					}else {
						printf("%d de %lu | DEBUG | THREAD ID %d SLEEP\n",connect, (long) pthread_self(),info.id);
					}
					
					/* Impression du contenu */
					pthread_mutex_lock(&mutexFile);
					lseek(fichierfd, 0, SEEK_SET);
					while(1){
						n = read(fichierfd, DATA_FILE,TAILLE);
						
						//printf("%lu | Lecture de data %d sur fichier %d\n",(long)pthread_self(),n,fichierfd);
						if (n < 0)
							perror("read ");
						
						//pthread_mutex_lock(&mutexSocket);
						n = write(connect, DATA_FILE, n);
						//pthread_mutex_unlock(&mutexSocket);
						//printf("%lu | Ecritre de data %d sur socket  %d\n",(long)pthread_self(),n,connect);
						if (n < 0){
							perror("write ");
							exit(EXIT_FAILURE);
						}
							
						
						if(n == 0)
							break;
									
						/* Ne pas oublier d'effacer le buffer */
						memset(DATA_FILE, 0, TAILLE);
					}
					pthread_mutex_unlock(&mutexFile);
					close(fichierfd);
					
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
							printf("%d | exécution finale du fichier : %s \n",getpid(),info.PATH_FILE);
							dup2(filsPipe[1], STDOUT_FILENO);
							dup2(filsPipe[1], STDERR_FILENO);
							close(filsPipe[0]);
							close(filsPipe[1]);
							execl(info.PATH_FILE, info.PATH_FILE, NULL);
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
								
								info.infoReq.ret_code = "200";
								
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
							read(localPipe[0], &val_Ecrite,sizeof(int)); /* Nombre d'octet écrit dans le fils*/
							info.infoReq.ret_code = "200";
							info.infoReq.total_size_send += *val_Ecrite;
						}else{ /*Dans le cas d'un echec de l'exécution */
							read(localPipe[0], &val_Ecrite,sizeof(int)); /* Nombre d'octet écrit : ici 0 */
							strcat(DATA, ret_code);
							strcat(DATA, " Internal Server Error\r\n\n"); 
							info.infoReq.ret_code = "505";
							info.infoReq.total_size_send +=write(connect, DATA, strlen(DATA));
						}
						close(fichierfd);
					} /* <- Fin else socket traitement */
				}/* <- Fin cas exécutable */
			}else if (bRepertory){
				//Ecriture des infos du répertoire
				printf("A FAIRE // Répertoire\n");
				
			}
						
		}else {		/* Traitement de l'erreur */
			printf("%d de %lu | Traitement code erreur : %d\n",connect,(long)pthread_self(),Errno);
			strcat(DATA, STD_REP);
			
			switch (Errno) {
				case ENOENT: strcat(DATA, "404 Not Found\r\n\n"); info.infoReq.ret_code = "404"; break;
				case EACCES: strcat(DATA, "403 Forbidden\r\n\n"); info.infoReq.ret_code = "403" ;break;
				default: strcat(DATA, "418 	I’m a teapot\r\n\n"); info.infoReq.ret_code = "418";
				
			}
			info.infoReq.total_size_send += write(connect, DATA, strlen(DATA));
		}
		
		
		/*Enregistrement des informations dans le fichier de log */
		recordToLogFile(&info.infoReq);
		pthread_mutex_lock(&mutexSocket);
		*nbThreadActif = *nbThreadActif-1;
		printf("%d de %lu | THREADREP DECREMENTATION %d\n",connect,(long)pthread_self(),*nbThreadActif);
		pthread_mutex_unlock(&mutexSocket);
		
		
		printf("%d de %lu | THREADREP FIN TRAITEMENT de %s \n",connect, (long)pthread_self(),info.FILE);
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
	
	if(strlen(str_request) == 0){
		return "text/plain";
	}
	
	/* On recherche l'expression régulière */
	FILE *file ;
	int bStrOk = 1;
	char line[256];
	char *strtType;
	char *strtContent;
	char *strtResult;
	char *saveptr1;
	
	/* Ouverture du fichier mime */
	if((mimefd = open("/etc/mime.types", O_RDONLY, S_IRUSR|S_IRGRP|S_IROTH)) == -1){
		//perror("ouverture mime par defaut ");
		/*Utilisation du mime du serveur */
		if((mimefd = open("./mime.types", O_RDONLY, S_IRUSR|S_IRGRP|S_IROTH)) == -1){
			perror("mime open");
			exit(-1);
		}
	}
	
	if((file = fdopen(mimefd, "r")) == NULL){
		perror("fdopen");
		pthread_exit((void *)0);
	} 
	
	
	while (fgets(line, sizeof(line), file)) {
		
		//printf("%s",line);
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
	
	/* Fermeture du fichier */
	fclose(file);
	close(mimefd);
	return ret;
}
/*
 *	traitement du signal alarm;
 *
 */
void traitSigAlarm(){
	kill(pidFils, SIGINT);
}

/*
 *	Fermeture normale du serveur;
 *
 */
void traitSignal(int sig){
	printf("\n Attente de la fin des threads\n");
	/* Fin des Threads */
	bThreadActif = 0;
	int i;

	for(i = 0;i<MAXCLIENT;i++)
		pthread_join(pthreadAlive[i], (void*)0);
	
	afficherConnexions();
	
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
	printf("PORT : %s\n", argv[1]);
	
	action.sa_handler = traitSignal;
	sigaction(SIGINT, &action, NULL);
	printf("Configuration signaux : OK\n");
	printf("PID du serveur principal : %d\n", getpid());
	printf("Dernier plus grand descripteur : %d\n",socketfd);
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
	
	pthreadAlive = malloc(sizeof(long)*MAXCLIENT);

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
			info = malloc(sizeof(struct infoRequete));
			/* Enregistrement du thread dans le tableau */
			info->connexionId = nbTotalConnexion;
			nbTotalConnexion++;
			printf("NOMBRE TOTAL DE CONNEXION : %d\n", nbTotalConnexion);
			printf("Numéro du descripteur : %d\n",connexion);
			pthread_mutex_unlock(&mutexConnexion);
			
			/* Informations pour traitement et journalisation */
			
			info->connectfd = connexion;
			struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&adr;
			info->ipAddr = pV4Addr->sin_addr;
			info->curtime = time (NULL);
			info->pid = getpid();
			
			printf("LANCEMENT %d\n",info->connectfd);
			//if(!(info->ipAddr.s_addr == infoSouvenir->ipAddr.s_addr)){
				
			pthread_create(&pidThread,NULL,(void*)funcTraitement,info);
			pthread_mutex_lock(&mutexConnexion);
			pthreadAlive[nbTotalConnexion-1] = pidThread;
			pthread_mutex_unlock(&mutexConnexion);
			//			*infoSouvenir = *info;
			//}else{
			//	printf("Empechement passage\n");	
			//}
				
			
		}else{
			//Message de rejet
			printf("PASSAGE ELSE\n");
			close(connexion);
		}
		pthread_mutex_unlock(&mutexConnexion);
		pthread_mutex_lock(&mutexConnexion);
		afficherConnexions();
		pthread_mutex_unlock(&mutexConnexion);
	}
}