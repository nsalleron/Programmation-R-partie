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

int idClient, idServeur;
struct myshm *shmClient, *shmServeur;
char nomClient[TAILLE_MESS];
char nomServeur[TAILLE_MESS];
char textToSend[TAILLE_MESS];



void funcThreadEcriture(void *arg){

	struct message messageClient;
	/* Phase d'enregistrement sur le serveur */
	sem_wait(&shmServeur->sem);
	messageClient.type =  1;
	strcpy(messageClient.content,nomClient);
	shmServeur->messages[shmServeur->write] = messageClient;
	shmServeur->write++;
	sem_post(&shmServeur->sem);


	printf("Fin d'enregisrement sur le serveur de %s\n",nomClient);


	while(1){
		//printf("Votre texte : ");
		//read(0,&textToSend,sizeof(textToSend));	
		scanf("%s",&textToSend);
		if(strlen(textToSend)!=0){
			//sem_wait(&shmServeur->sem);
			sem_wait(&shmClient->sem);
			messageClient.type = 2;
			strcpy(messageClient.content,textToSend);
		
			shmServeur->messages[shmServeur->write] = messageClient;
			shmServeur->write++;
						
			shmClient->nb++;
			sem_post(&shmClient->sem);
			sem_post(&shmServeur->sem);
		}	
	}
}

void funcThreadLecture(void *arg){

	struct message messageClient;
	
	while(1){
		
		sem_wait(&shmClient->sem);
		if(shmClient->write != 0){
			messageClient = shmClient->messages[shmClient->write-1]; 
			printf("Recu : %s\n",messageClient.content);
			shmClient->read++;
			shmClient->nb++;
			shmClient->write--;
		}
		sem_post(&shmClient->sem);

			
	}
}

void sigIntTrt(int sig){

	struct message messageClient;
	
	//On se désenregistre
	sem_wait(&shmServeur->sem);

	messageClient.type =  3;
	strcpy(messageClient.content,nomClient);
	shmServeur->messages[shmServeur->write] = messageClient;
	shmServeur->write++;

	sem_post(&shmServeur->sem);

	munmap(&shmClient,sizeof(struct myshm));
	shm_unlink(nomClient);
	printf("Fin de la conversation \n");
	exit(0);

}



int main(int argc,char *argv[]){

	pthread_t threadEcriture, threadLecture;
	sigset_t ens;
	sigfillset(&ens);
	sigdelset(&ens,SIGINT);
	struct sigaction action;
	action.sa_mask = ens;
	action.sa_flags = 0;
	action.sa_handler =sigIntTrt;
	sigaction(SIGINT,&action,NULL);


	shm_unlink(argv[1]);

	printf("Valeur argv1 : %s, argv2 : %s\n",argv[1], argv[2]);
	/* Segment de mémoire client */
	if ((idClient = shm_open(argv[1],O_RDWR | O_CREAT,0600)) == -1) {
		perror("Echec de l'allocation du descripteur\n");
		exit(errno);
	}
	/* Allocation pour stocker 1 entiers*/
	if(ftruncate(idClient,sizeof(struct myshm)) == -1){
		fprintf(stderr, "Echec d'allocation ftruncate\n");
	}
	/* Segmentation pour stocker 1 entiers*/
	if ((shmClient = mmap(NULL, sizeof(struct myshm), PROT_READ|PROT_WRITE, MAP_SHARED, idClient, 0)) == MAP_FAILED){
		fprintf(stderr, "Echec de l'allocation de la ressource partagee\n");
		//exit(errno);
	}
	printf("fin ouverture client\n");

	strcpy(nomClient,argv[1]);
	strcpy(nomServeur,argv[2]);

	/* Segment de mémoire serveur */
	if ((idServeur = shm_open(argv[2],O_RDWR,0)) == -1) {
		perror("Echec de l'allocation du descripteur\n");
		exit(errno);
	}
	/* Segmentation pour stocker 1 entiers*/
	if ((shmServeur = mmap(NULL, sizeof(struct myshm), PROT_READ|PROT_WRITE, MAP_SHARED, idServeur, 0)) == MAP_FAILED){
		fprintf(stderr, "Echec de l'allocation de la ressource partagee\n");
		//exit(errno);
	}

	sem_init(&shmClient->sem,1,1);

	pthread_create(&threadEcriture,(void *)0,(void*)funcThreadEcriture,(void *)0);
	pthread_create(&threadLecture,(void *)0,(void*)funcThreadLecture,(void *)0);
	pthread_join(threadEcriture,(void*)0);

}






