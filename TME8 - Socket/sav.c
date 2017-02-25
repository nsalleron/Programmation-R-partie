//Lorsqu’une connexion s’ouvre, il lit la première ligne envoyée et considère que c’est le nom d’un fichier
n = read(connexion,&CMD_BRUT,TAILLE);
if (n < 0)
	perror("ERROR in recvfrom");
	
CMD = strtok(CMD_BRUT, " ");
printf("CMD : %s",CMD);
NOM_FICHIER = strtok(NULL, " ");
printf("CMD : %s",NOM_FICHIER);
	
if(!strcmp(CMD,"LIST")){
	
	pipe(tube); //Création d'un tube pour communication (récupération du résultat de ls);
	if(fork() == 0){
		dup2(tube[1], STDOUT_FILENO);
		dup2(tube[1], STDERR_FILENO); //On place également l'erreur dans le flux de sortie;
		close(tube[0]);
		close(tube[1]);
		execlp("ls", "ls","-l",NULL);
		perror("execlp");
		exit(2);
		
	}
	close(1);
	wait(NULL); //Attente du fils
	read(tube[0], DATALIST, 4*TAILLE);
	
	//On envoi la réponse au client maintenant
	n = write(socketfd, DATA, 4*TAILLE);
	if (n < 0) 
		perror("ERROR in sendto");
	
	
}else if(!strcmp(CMD,"UPLOAD")){
	//il crée alors dans son répertoire d’exécution un fichier vide portant ce nom.
	if((fichierfd = open(NOM_FICHIER, O_CREAT|O_WRONLY, 0666)) == -1){
		perror("open ");
		close(connexion);
		exit(-1);
	}
	
	//Il lit ensuite les données transmises jusqu’à la fin de la connexion, et les recopie dans le fichier créé précédemment.
	while(1){
		n = read(connexion, &DATA,4*TAILLE);
		printf("Impression data %d\n",n);
		if (n < 0)
			perror("ERROR in recvfrom");
		if(n == 0)
			break;
		if(write(fichierfd, DATA, 4*TAILLE)==-1)
			perror("write ");
		/* Ne pas oublier d'effacer le buffer */
		memset(DATA, 0, 4*TAILLE);
	}
	
	printf("Fin d'écriture du fichier \n");
	
}else if(!strcmp(CMD,"DOWNLOAD")){
	
	/*Ouverture du fichier */
	fichierfd = open(NOM_FICHIER, O_RDONLY, 0666);
	
	/*Vidage et copie dans le buffer pour le nom */
	memset(NOM_FICHIER, 0, TAILLE);
	strcpy(NOM_FICHIER, argv[3]);
	
	/*Ecriture sur la socket*/
	n = write(socketfd, NOM_FICHIER, TAILLE);
	if (n < 0) 
		perror("ERROR in sendto");
	
	/* Boucle de traitement */
	while((n=read(fichierfd,&DATA,4*TAILLE)) != 0){

		n = write(socketfd, DATA, 4*TAILLE);
		if (n < 0) 
			perror("ERROR in sendto");
			
		/* Ne pas oublier de remettre à 0 le buffer */
		memset(DATA, 0, 4*TAILLE);
	}
	shutdown(socketfd, SHUT_RDWR);
	close(socketfd);
	close(fichierfd);
	
}