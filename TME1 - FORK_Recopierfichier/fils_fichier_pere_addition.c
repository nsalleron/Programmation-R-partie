#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//Ecrire en C un programme prenant en argument un nombre n et un nom de fichier. Le processus principal doit créer n processus fils, à l’aide de fork. Chaque processus fils produit une valeur aléatoire qu’il insère dans le fichier donné en 2e argument, à destination du processus principal. La valeur aléatoire est calculée par :
//
//	(int) (10*(float)rand()/ RAND_MAX)
//
//	De son côté, le processus principal doit attendre la terminaison de tous ses fils, puis extraire toutes les valeurs du fichier pour ensuite les additionner et enfin afficher la somme résultante. On pourra utiliser waitpid appliqué aux résultat de fork pour attendre les terminaisons, puis utiliser lseek. On contrôlera les cas d’erreurs comme à l’exercice précédent.
//
//	Exemple d'appel :
//	$PWD/bin/remonte 8 aleas

int desc_cible = 0, N = 0, i = 0;
pid_t *tabFils;
char *ValeurLecture;
FILE *ptFile;
int valTot;
char c;

int main(int argc, char *argv[]) {
	
	//Vérification du nombre d'arguments
	if(argc != 3){
		perror("-> Le nombre de paramètres est incorrect.\n"); return EXIT_FAILURE;
	}

	//Récupération du nombre de fils
	N = atoi(argv[1]);
	//Création d'un tableau pour connaitre le PID de mes fils
	tabFils = malloc(sizeof(pid_t)*N);
	
	//Vérification si l'ouverture est correcte
	if( (desc_cible = open(argv[2],O_CREAT|O_RDWR,0700)) == -1){
		perror("-> Problème d'ouverture en lecture du fichier \n");
		return EXIT_FAILURE;
	}
	//Ouverture pour pouvoir manipuler des caractères simplement
	if((ptFile = fdopen(desc_cible, "w+"))==NULL){
		perror("-> Probleme fdopen : ");
		return EXIT_FAILURE;
	}
	
	for (i=0;i<N;i++) {
		//Fork
		if((tabFils[i] = fork()) == -1){
			perror("-> Problème de fork");
			return EXIT_FAILURE;
		}
		//Dans le fils
		if(tabFils[i]==0){
			
			//printf("Fils : %d, père : %d\n", getpid(),getppid());
			
			//Génération de la valeur aléatoire
			int valeur = (10*(float)rand()/ RAND_MAX);
			//printf("-> valeur : %d\n",valeur);
			//Ecriture
			if(fprintf(ptFile, "%d", valeur)==-1){
				perror("-> Problème de fprintf :");
				fclose(ptFile);
				return EXIT_FAILURE;
			}
			return EXIT_SUCCESS;
		}
	}
	
	
	//Attente de l'ensemble des fils
	for (i=0;i<N;i++) {
		waitpid(tabFils[i],NULL,0);
	}
	
	//Placement en début de fichier
	lseek(desc_cible, 0, SEEK_SET);
	
	//Lecture et addition entier par entier
	while ((c = fgetc(ptFile)) != EOF) {
		//printf("%c\n",c);
		valTot += (c - '0');
	}
	
	fclose(ptFile);
	printf("=>Valeur totale : %d\n",valTot);
	
	return EXIT_SUCCESS;
	
}