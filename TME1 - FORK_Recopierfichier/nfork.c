#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//A l’aide de la fonction standard fork, définir la fonction
//
//int nfork (int n)
//qui crée n processus fils et renvoie :
//
//le nombre de processus fils créés, pour le processus père ;
//0 pour les processus fils ;
//-1 dans le cas où aucun fils n’a été créé.
//Si la création d’un processus fils échoue, la fonction n’essaie pas de créer les autres fils. Dans ce cas, la fonction renvoie le nombre de fils qu’elle a pu créer (ou –1, si aucun fils n’a été créé). On pourra compléter cette fonction par une fonction main l’appliquant à un entier pas trop grand pour la tester.
//
//Exemple d'appel :
//$PWD/bin/nfork

int nfork(int n){

	int nbFils = 0;
	pid_t fils;

	while( nbFils < n){
		if((fils = fork()) ==-1){
			if (nbFils == 0) {
				return -1;
			}else {
				return nbFils;
			}
			perror("fork"); exit(1);
		}else if(fils ==0){
			printf("fils %d\n",nbFils);
			exit(1);
		}
		nbFils++;
	}
	
	return 0;
}


int main(int argc,char *argv[]){
	if(argc != 2){
		printf("Pas assez d'arguments");
		exit(0);
	}else{

		printf("Début\n");
		nfork(atoi(argv[1]));
		int n = 0;
		while(n<atoi(argv[1])){
			wait(NULL);
			n++;
		}
		printf("Fin\n");
		
		
	}
}