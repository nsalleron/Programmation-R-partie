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
int main(int argc, char *argv[]){

  int id, n, i, total, res;
  int *ptr = NULL;
  int *pids;

  n = (argc < 2) ? 0 : strtol(argv[1], NULL, 10);
  if (n <= 0) {
    fprintf(stderr, "Usage: %s nombre\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Demande un descripteur de ressource partagee, eventuellement deja creee. */
  
  if ((id = shm_open("monshm",O_RDWR | O_CREAT,0600)) == -1) {
    perror("Echec de l'allocation du descripteur\n");
    exit(errno);
  }

  /* Allocation pour stocker n entiers*/
  if(ftruncate(id,sizeof(int)*n) == -1){
    fprintf(stderr, "Echec d'allocation ftruncate\n");
  }
  /* Allocation pour stocker n entiers*/
  if ((ptr = mmap(NULL, sizeof(int)*n, PROT_READ|PROT_WRITE, MAP_SHARED, id, 0)) == MAP_FAILED){
    fprintf(stderr, "Echec de l'allocation de la ressource partagee\n");
    //exit(errno);
  }
  
  //Une fois shm_open ftruncate mmap fait, la mémoire est alloué et segmenter correctement. 
  
  pids = malloc(n * sizeof(int));

  for(i=0; i<n; i++){
    int pid = fork();
    if (pid == -1){
      perror("fork");
      return -1;
    } else if (pid) {
      pids[i] = pid;
    } else {
      srand(time(NULL)*i);
      /* Ecriture dans la ressource partagee */
      ptr[i] = (int) (10*(float)rand()/ RAND_MAX);
      exit(EXIT_SUCCESS);
    }
  }
  for(i=0; i<n; i++){
    int status;
    waitpid(pids[i], &status, 0);
  }

  total = 0;

  for(i=0; i<n; i++){
    /* Lecture dans la ressource partagee */
    res = ptr[i];
    printf("pid %d envoie %d\n", pids[i], res);
    total += res;
  }

  free(pids);

  printf("total: %d\n", total);

  /* Liberation de la ressource */
  munmap(&res, sizeof(int)*n);
  
  if (shm_unlink("monshm")) {
    fprintf(stderr, "Ressource partagee mal rendue\n");
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
