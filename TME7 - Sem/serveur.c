#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "chat_common.h"

int idServeur, idClient;
struct myshm *shmServeur, *shmClient;
struct message messageClient;
char tabClient[MAX_USERS][TAILLE_MESS];
int i = 0, nbUserCourant = 0, emplacementVide = 0;
char utilisateur[TAILLE_MESS];

int main(int argc,char *argv[]){

	shm_unlink(argv[1]);

	/* Segment de mémoire serveur */
	if ((idServeur = shm_open(argv[1],O_RDWR| O_EXCL| O_CREAT,0777)) == -1) {
		perror("Echec de l'allocation du descripteur\n");
		exit(errno);
	}
	/* Allocation pour stocker 1 entiers*/
	if(ftruncate(idServeur,sizeof(struct myshm)) == -1){
		fprintf(stderr, "Echec d'allocation ftruncate\n");
	}
	/* Segmentation pour stocker 1 entiers*/
	if ((shmServeur = mmap(NULL, sizeof(struct myshm), PROT_READ|PROT_WRITE, MAP_SHARED, idServeur, 0)) == MAP_FAILED){
		fprintf(stderr, "Echec de l'allocation de la ressource partagee\n");
		//exit(errno);
	}
	
	sem_init(&shmServeur->sem,1,1);

	printf("Fin d'initialisation du serveur.\n");
	while(1){
		sem_wait(&shmServeur->sem);
		
		while(shmServeur->write != 0){ //Traitement de la requête 
		
			//Récupération du message
			messageClient = shmServeur->messages[shmServeur->write-1];	
			
			switch(messageClient.type){

				case 1: //Connexion
					printf("Connexion de %s!\n",messageClient.content);
					printf("Nombre d'utilisateur : %d\n",nbUserCourant);
					
					//Ajout de l'utilisateur
					for(i = 0 ; i<nbUserCourant;i++){
						if(!strcmp(tabClient[i],"")){
							emplacementVide = 1;
							strcpy(tabClient[i],messageClient.content);
						}
					}
					if(!emplacementVide){
						strcpy(tabClient[nbUserCourant],messageClient.content);
						nbUserCourant++;
					}
						
					//Affichage de l'utilisateur
					for(i=0;i<nbUserCourant;i++)
						printf("Utilisateur connecté: %s\n",tabClient[i]);
					
					break;
				case 2:	//Diffusion des messages vers les utilisateurs connectés.
					
					for(i = 0;i<nbUserCourant;i++){
						
						if(!strcmp(tabClient[i],""))	//Dans le cas d'un troue dans la table.
							continue;

						printf("Envoi vers \"%s\" de :%s\n",tabClient[i],messageClient.content);
						
						if ((idClient = shm_open(tabClient[i],O_RDWR,0)) == -1) {
							perror("Echec de l'allocation du descripteur\n");
							exit(errno);
						}
					
						if ((shmClient = mmap(NULL, sizeof(struct myshm), PROT_READ|PROT_WRITE, MAP_SHARED, idClient, 0)) == MAP_FAILED){
							fprintf(stderr, "Echec de l'allocation de la ressource partagee\n");
							//exit(errno);
						}

						sem_wait(&shmClient->sem);
							shmClient->messages[shmClient->write] = messageClient;
							shmClient->write++;
							shmClient->read++;
						sem_post(&shmClient->sem);
						shmServeur->read++;
						shmServeur->nb++;

					}//for
					break;
				case 3:	//Déconnexion
				
					strcpy(utilisateur,messageClient.content);
					
					printf("Déconnexion de: %s\n",utilisateur);
					
					for(i=0;i<nbUserCourant;i++)
						if(!strcmp(tabClient[i],utilisateur)){	//On a trouvé l'utilisateur
							printf("Suppression de %s\n",utilisateur);
							for(;i<nbUserCourant-1;i++){
								printf("Déplacement de %s vers %s\n",tabClient[i+1],tabClient[i]);
								strcpy(tabClient[i],tabClient[i+1]);
							}
							memset(tabClient[i],0,sizeof(tabClient[i]));
						}
					break;
			}
			shmServeur->write--;
		}
		sem_post(&shmServeur->sem);
	}
}






