#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
/*
pause : le processus appelant se met en attente de la réception d'un signal non ignoré et non masque.
Quand il recoit un tel signal : 
	- le processus se termine si le handler associé au signal délivré est SIG_DFL et que l'effet de ce handler est de terminer le processus.
	- le processus excute le handler correspondant au signal délivré si il est capter, puis reprend son exécution en séquence. 
	
=> pause ne permet ni d'attendre l'arrrivée d'un signal ni au reveil de savoir quel signal a réveillé le processus. 
	=>Pour le savoir il faut récupérer cette information par l'intermédiaire du ou des handler.
*/

int tabSig[] = {SIGINT, SIGQUIT, SIGUSR1, SIGUSR2};
int ind, numSig,rep;
struct sigaction action;

/*handler des signaux du père
*/
void handPere(int sig){
	printf("Signal %d reçu par le processus père\n",sig);
}
int main(int argc, char *argv[]) {
	srand(time(NULL));
	action.sa_handler = handPere; // On indique la fonction à exécuter quand on recoit un signal
	for(ind = 0;ind<4;ind++) sigaction(tabSig[ind],&action,NULL);
	if(fork() ==0) { //processus fils
		//while (1) {
			numSig = rand()%4;
			printf("signal %d envoyé par le processus fils\n",tabSig[numSig]);
			kill(getppid(),SIGSTOP);
			sleep(2);
			kill(getppid(),SIGCONT);

		//}
		
		exit(0);
	}
	//while (1) {
		rep = pause();
		printf("pause() = %d \n",rep);
		//perror("pause");
	//}
	

}