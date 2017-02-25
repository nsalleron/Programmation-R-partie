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

int main(int argc, char *argv[]) {
	char buf[10];
	int fd1, fd2;
	fd1 = open("./fichier1",O_RDONLY);
	read(fd1,buf,2);
	if(fork() == 0){
		fd2 = open("./fichier1",O_RDWR);
		write(fd2,"1234",5);
		close(fd1);close(fd2);
		exit(1);
	}
	wait(NULL);
	read(fd1,buf,2);
	buf[2] = 0;
	printf("%s",buf);
	close(fd1);

	return 0;
}
