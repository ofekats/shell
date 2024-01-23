#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>

void handler(int num)
{
	write(STDOUT_FILENO, "", 0);
}

int main()
{
	int i, amper, retid, status;
	char *argv[10];
	char command[1024];
	char *token;
	char prompt[256] = "\x1b[35mMoriyaEsterAndOfek~stshell\x1b[0m";

	signal(SIGINT, handler);
	while (1)
	{
		printf("\x1b[35m%s: \x1b[0m", prompt);
		fgets(command, 1024, stdin);
		command[strlen(command) - 1] = '\0'; // replace \n with \0
		if (strcmp(command, "exit") == 0 || strcmp(command, "exit()") == 0)
		{
			return 0;
		}

		else if (strncmp(command, "prompt = ", 9) == 0)
        {
            // Change prompt command
            strncpy(prompt, command + 9, sizeof(prompt) -1);
            prompt[sizeof(prompt) - 1] = '\0'; // Ensure null-termination
            continue; 
        }


		/* parse command line */
		i = 0;
		token = strtok(command, " ");
		while (token != NULL)
		{
			argv[i] = token;
			token = strtok(NULL, " ");
			i++;
		}
		argv[i] = NULL;

		/* Is command empty */
		if (argv[0] == NULL)
			continue;

        /* Does command line end with & */ 
        if (! strcmp(argv[i - 1], "&")) {
            amper = 1;
            argv[i - 1] = NULL;
        }
        else 
            amper = 0; 

		/* for commands not part of the shell command language */
		int id = fork();
		if (id < 0)
		{
			perror("Error");
			return 1;
		}
		if (id == 0)
		{
			signal(SIGINT, SIG_DFL); // Reset SIGINT handler to default (ignore)
			int i = 0;
			while (argv[i] != NULL)
			{
				// if we have >
				if (strcmp(argv[i], ">") == 0)
				{
					char *filename = argv[i + 1];
					int fd1 = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
					if (fd1 < 0)
					{
						perror("Error");
						return 1;
					}
					if (dup2(fd1, 1) < 0) // 1 == stdout
					{
						perror("Error");
						return 1;
					}
					close(fd1);
					argv[i] = NULL;
					break;
				}
                if (strcmp(argv[i], "2>") == 0)
				{
					char *filename = argv[i + 1];
					int fd1 = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
					if (fd1 < 0)
					{
						perror("Error");
						return 1;
					}
					if (dup2(fd1, 2) < 0) // 2 == stderr
					{
						perror("Error");
						return 1;
					}
					close(fd1);
					argv[i] = NULL;
					break;
				}
				// if we have >>
				if (strcmp(argv[i], ">>") == 0)
				{
					char *filename = argv[i + 1];
					int fd1 = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
					if (fd1 < 0)
					{
						perror("Error");
						return 1;
					}
					if (dup2(fd1, 1) < 0) // 1 == stdout
					{
						perror("Error");
						return 1;
					}
					close(fd1);
					argv[i] = NULL;
					break;
				}
				// if we have |
				if (strcmp(argv[i], "|") == 0)
				{
					int fd[2];
					if (pipe(fd) < 0)
					{
						perror("Error");
						return 1;
					}
					int id2 = fork();
					if (id2 < 0)
					{
						perror("Error");
						return 1;
					}
					if (id2 == 0)
					{ 
						//the first command: <first> | <second>
						close(fd[0]);
						int j = 0;
						char *argv1[10];
						while (strcmp(argv[j], "|"))
						{
							argv1[j] = argv[j];
							j++;
						}
						argv1[j] = NULL;
						if (dup2(fd[1], 1) < 0) // stdout of grandchild == fd[1] -> write to pipe
						{
							perror("Error");
							return 1;
						}
						close(fd[1]);
						execvp(argv1[0], argv1);
					}
					else
					{ 
						//the second command: <first> | <second>
						int j = 0;
						int k =i+1;
						while (argv[k] != NULL)
						{
							argv[j] = argv[k];
							j++;
							k++;
						}
						i = -1;
						argv[j] = NULL;
						close(fd[1]);
						if (dup2(fd[0], 0) < 0) // stdin of child == fd[0] -> read of pipe
						{
							perror("Error");
							return 1;
						}
						close(fd[0]);
						wait(NULL);
					}
				}
				i++;
			}
			execvp(argv[0], argv);
		}
		/* parent continues here */
        if (amper == 0)
            retid = wait(&status);
	}
}