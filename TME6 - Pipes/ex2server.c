#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


//On se propose de reprendre l’exercice précédent, mais au moyen d’un processus serveur dont les clients sont sans lien de parenté avec lui. L’utilisateur doit pouvoir lancer le serveur puis, éventuellement à travers une autre fenêtre de terminal, lancer un programme client qui s’adresse au serveur grâce à un tube nommé, fourni par la fonction C mkfifo.
//
//Dans cette partie, il s’agit d’écrire le serveur. Il prend sur la ligne de commande le nom du tube à créer, et le crée. Le serveur se met à l’écoute sur ce tube et affiche la transcription en majuscules dans son propre flux de sortie. Il doit pouvoir être interrompu par un ^C, et doit alors fermer le tube nommé et le détruire.
//
//Exemple d'appel :
//$PWD/bin/serveur_maj minmaj &

int fdread;
char *file;
int ecoute = 1;
char c;


void *func(int sig){
	printf("Interruption du serveur\n");
	close(fdread);
	unlink(file);
	ecoute = 0;
	return 0;
}


int main(int argc, char *argv[]) {
	
	if(argc<2){
		perror("Arg invalide : \n");
		exit(2);
	}
	
	//Copy de l'@ du fichier
	file = malloc(sizeof(argv[1]));
	strcpy(file, argv[1]);
	
	//Signaux
	sigset_t sig;
	sigfillset(&sig);
	sigdelset(&sig, SIGINT);
	sigprocmask(SIG_SETMASK, &sig, &sig); //Retour ancien masque
	
	struct sigaction action;
	action.sa_flags = 0;
	action.sa_mask = sig;
	action.sa_handler = (void *)func;
	sigaction(SIGINT, &action, 0);
	
	if(mkfifo(argv[1], S_IRUSR|S_IWUSR) == -1)
		perror("mkfifo");
	
	if((fdread = open(argv[1],O_RDONLY)) == -1){
		perror("open ");
		exit(2);
	}

	while(ecoute){
		read(fdread, &c, sizeof(char));
		printf("%c",toupper(c));
	}
	
}