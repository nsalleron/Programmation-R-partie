
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024
int main(int argc, char **argv) {
	
	int sockfd, portno, n;
	socklen_t serverlen;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	char *hostname;
	char buf[BUFSIZE];

	/* vérification de la commande */
	if (argc != 3) {
		 fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
		 exit(0);
	}
	hostname = argv[1];
	portno = atoi(argv[2]);

	/* creation du socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
			perror("socket");

	/* récupération nom client */
	server = gethostbyname(hostname);
	if (server == NULL) {
			fprintf(stderr,"host %s\n", hostname);
			exit(0);
	}

	/* création adresse serveur */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
	serveraddr.sin_port = htons(portno);

	while(1){
		/* message */
		bzero(buf, BUFSIZE);
		printf("message: ");
		//fgets(buf, BUFSIZE, stdin);
		read(STDIN_FILENO,buf,BUFSIZE);

		/* envoie */
		serverlen = sizeof(serveraddr);
		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*) &serveraddr, serverlen);
		if (n < 0) 
			perror("ERROR in sendto");
			
		/* On réinitialise le serveur à 0 */
		bzero(buf, BUFSIZE);
		
		/* On reçoit et l'on précise la taille du buffer /!\ */
		n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr*) &serveraddr, &serverlen);
		if (n < 0) 
			perror("ERROR in recvfrom");
		printf("Echo from server: %s\n", buf);

	}
	return 0;




	
}