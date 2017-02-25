#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

//1 Tube et majuscules
//
//On souhaite écrire un programme qui mette en majuscules les chaînes de caractères entrées par l’utilisateur via le terminal. Ce programme lance 2 processus :
//
//le processus père créé un tube avec la fonction C pipe et un processus fils, récupère les messages utilisateur en les lisant sur l’entrée standard, puis les transmet à son fils ;
//le processus fils lit les messages de son père, les transcrit en majuscules avec la fonction C toupper, puis les redirige vers la sortie standard.
//Exemple d'appel :
//echo abcd.ext | $PWD/bin/pipe_maj

int main(int argc, char *argv[]) {
	
	int tube[2];
	char toto;
	int fin;
	pipe(tube);
	
	pid_t fils = 0;
	
	if((fils = fork()) == -1){
		perror("fork : \n");
	}
	if(fils != 0){//père
		close(tube[0]);
		while(!fin)
			if(scanf("%c",&toto)!=0){
				write(tube[1], &toto, sizeof(char));
			}
		close(tube[0]);
		close(STDIN_FILENO);
	}else{
		close(tube[1]);
		dup2(tube[0], STDIN_FILENO);
		while(!fin)
			if(read(STDIN_FILENO, &toto, sizeof(char))!=0){
				printf("%c",toupper(toto));
				fflush(stdout);
			}
		close(tube[0]);
		close(STDIN_FILENO);
		printf("fin ! \n");
	}
		
	
}