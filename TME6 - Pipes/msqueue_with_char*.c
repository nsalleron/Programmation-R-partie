#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <mqueue.h>

int main()
{
	srand(time(NULL));

	mqd_t mqd;

	struct mq_attr attr = {.mq_maxmsg = 10, .mq_msgsize = sizeof(int)};

	int i;
	int pid;
	int status;
	int num_children = atoi(argv[1]);

	int buffer[atributos.mq_msgsize];

	for (i = 0; i < num_children; i++)
	{
		if ((pid = fork() == 0))
		{
			int prio = 3;

			char* msg = 10;

			strncpy (buffer, msg, sizeof(buffer));

			if ((mqd = mq_open("/queue.txt", O_CREAT | O_WRONLY, 0666, &atributos)) == -1)
			{
				perror("child mq_open");
				exit(1);
			}

			if (mq_send(mqd, buffer, sizeof(buffer), prio) == -1)
			{
				perror("mq_send");
				exit(1);
			}

			mq_close(mqd);

			exit(0);
		}

	}

	// parent

	if ((mqd = mq_open("/queue.txt", O_CREAT | O_RDONLY, 0666, &atributos)) == -1)
	{
		perror("parent mq_open");
		exit(1);
	}

	int priority;

	for (int i = 0; i < num_children; ++i)
	{
		if (mq_receive(mqd, buffer, sizeof(buffer), &priority) == -1)
		{
			perror("mq_recieve");
			exit(1);
		}

		printf("Received (%s): %s\n", (priority == 0) ? "NORMAL" : "URGENT", buffer);

		pid_t childpid;

		if ((childpid = waitpid(-1, &status, 0)) > 0)
		{
			if (WIFEXITED(status))
				printf("PID %d exited normally.  Exit status: %d\n",
					   childpid, WEXITSTATUS(status));
			else
				if (WIFSTOPPED(status))
					printf("PID %d was stopped by %d\n",
						   childpid, WSTOPSIG(status));
				else
					if (WIFSIGNALED(status))
						printf("PID %d exited due to signal %d\n.",
							   childpid,
							   WTERMSIG(status));
		}
	}

	mq_close(mqd);
}