#if !defined(_XOPEN_SOURCE) && !defined(_POSIX_C_SOURCE)
#if _STDC_VERSION_ >= 199901L
#define _XOPEN_SOURCE 600 /* SUS v3, POSIX 1003.1 2004 (POSIX 2001 + Corrigenda) */
#else
#define _XOPEN_SOURCE 500 /* SUS v2, POSIX 1003.1 1997 */
#endif					  /* _STDC_VERSION_ */
#endif					  /* !_XOPEN_SOURCE && !_POSIX_C_SOURCE */

#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>

int flag_if;

void handler()
{
	write(STDOUT_FILENO, "", 0);
	flag_if = 0;
	printf("\nYou typed Control-C!\n");
}

// Structure to store variables
typedef struct
{
	char name[256];
	char value[1024];
} Variable;

Variable variables[100]; // Array to store variables
int variableCount = 0;	 // Counter for the number of variables

// Function to find a variable by name
Variable *findVariable(char *name)
{
	for (int i = 0; i < variableCount; i++)
	{
		if (strcmp(variables[i].name, name) == 0)
		{
			return &variables[i];
		}
	}
	return NULL;
}

int addVariable(char *name, char *value)
{
	if (variableCount < 100)
	{
		strncpy(variables[variableCount].name, name, sizeof(variables[variableCount].name) - 1);
		variables[variableCount].name[sizeof(variables[variableCount].name) - 1] = '\0'; // Ensure null-termination
		strncpy(variables[variableCount].value, value, sizeof(variables[variableCount].value) - 1);
		variables[variableCount].value[sizeof(variables[variableCount].value) - 1] = '\0'; // Ensure null-termination
		variableCount++;
		return 0;
	}
	return -1;
}

int main()
{
	int i, amper, status;
	flag_if = 0;
	char *argv[10];
	char command[1024];
	char new_command[1024];
	char command_thenOrElse[1024];
	char *token;
	char prompt[256] = "\x1b[35mHello\x1b[0m";
	char last_command[1024] = "";

	struct sigaction sa;
	sa.sa_handler = handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		perror("error");
		return EXIT_FAILURE;
	}
	while (1)
	{
		if (flag_if) // print > if we are in if
		{
			printf("\x1b[35m> \x1b[0m");
		}
		else
		{
			printf("\x1b[35m%s: \x1b[0m", prompt);
		}
		fgets(command, 1024, stdin);
		command[strlen(command) - 1] = '\0'; // replace \n with \0

		if (strcmp(command, "!!") == 0)
		{
			// Repeat the last command
			if (strlen(last_command) > 0)
			{
				strcpy(command, last_command);
			}
			else
			{
				printf("No previous command to repeat.\n");
				continue;
			}
		}
		strcpy(last_command, command);

		if (flag_if == 1)
		{ // after if search for then
			if (strncmp(command, "then", 4) == 0)
			{
				flag_if = 2;
			}
			else
			{
				printf(" there are no 'then'\n");
			}
			continue;
		}

		if (flag_if == 2)
		{ // after then, save the command only if the if-command was 0
			flag_if = 3;
			if (status == 0)
			{
				strncpy(command_thenOrElse, command, sizeof(command_thenOrElse) - 1);
			}
			continue;
		}
		if (flag_if == 3)
		{ // after then+command, search for else
			if (strncmp(command, "else", 4) == 0)
			{
				flag_if = 4;
				continue;
			}
			else
			{
				flag_if = 5; // after then+command, if there is no else go to search fi
			}
		}

		if (flag_if == 4)
		{ // after else, save the command only if the if-command was not success
			flag_if = 5;
			if (status != 0)
			{
				strncpy(command_thenOrElse, command, sizeof(command_thenOrElse) - 1);
			}
			continue;
		}
		if (flag_if == 5)
		{ // search for fi
			if (strncmp(command, "fi", 2) == 0)
			{
				flag_if = 0;
				strncpy(command, command_thenOrElse, sizeof(command) - 1);
				command_thenOrElse[0] = '\0';
			}
			else
			{
				printf(" there are no 'fi'\n");
				continue;
			}
		}

		if (strncmp(command, "if ", 3) == 0)
		{ // if there is if in the command
			flag_if = 1;
			// remove if from command
			strncpy(command, command + 3, sizeof(command) - 1);
		}

		/* parse command line */
		i = 0;
		strcpy(new_command, command);
		token = strtok(new_command, " ");
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

		if (strcmp(command, "exit") == 0 || strcmp(command, "exit()") == 0 || strcmp(command, "quit") == 0)
		{
			return 0;
		}
		else if (strncmp(command, "prompt = ", 9) == 0)
		{
			// Change prompt command
			strncpy(prompt, command + 9, sizeof(prompt) - 1);
			prompt[sizeof(prompt) - 1] = '\0'; // Ensure null-termination
			continue;
		}

		else if (strncmp(command, "echo $?", 7) == 0)
		{
			// Print the exit status of the last command
			printf("%d\n", status);
			continue;
		}

		else if (strncmp(command, "cd ", 3) == 0)
		{
			// Change current working directory
			char *new_directory = command + 3;
			if (chdir(new_directory) != 0)
			{
				perror("Error");
			}
			continue;
		}

		else if (strncmp(command, "read ", 5) == 0)
		{
			// Read command
			char *varName = command + 5;
			Variable *var = findVariable(varName);
			if (var != NULL)
			{
				fgets(var->value, sizeof(var->value), stdin);
				// Remove newline character if present
				size_t len = strlen(var->value);
				if (len > 0 && var->value[len - 1] == '\n')
				{
					var->value[len - 1] = '\0';
				}
			}
			else
			{
				// New variable, add it to the array
				char value[1024];
				fgets(value, sizeof(value), stdin);
				// Remove newline character if present
				size_t len = strlen(value);
				if (len > 0 && value[len - 1] == '\n')
				{
					value[len - 1] = '\0';
				}
				if (addVariable(varName, value) != 0)
				{
					printf("Too many variables, cannot add more.\n");
				}
			}
			continue;
		}

		else if (strchr(command, '$') != NULL)
		{
			char *equalSign = strchr(command, '=');
			if (equalSign != NULL)
			{
				// Variable assignment
				*equalSign = '\0'; // Split the string at '='
				char *varName = strtok(command, " ");
				char *varValue = equalSign + 2; // equalSign is \0, equalSign + 1 is space
				Variable *var = findVariable(varName + 1);
				if (var != NULL)
				{
					// Variable already exists, update its value
					strncpy(var->value, varValue, sizeof(var->value) - 1);
					var->value[sizeof(var->value) - 1] = '\0'; // Ensure null-termination
				}
				else
				{
					// New variable, add it to the array
					if (addVariable(varName + 1, varValue) != 0)
					{
						printf("Too many variables, cannot add more.\n");
					}
				}
				continue;
			}
			// Variable substitution
			int j = 0;
			while (argv[j] != NULL)
			{

				if (strchr(argv[j], '$') != NULL)
				{

					char *varName = argv[j]; // poiter to the char after the $ (name of the var)
					Variable *var = findVariable(varName + 1);
					if (var != NULL)
					{
						argv[j] = var->value;
					}
					else
					{
						printf("Variable '%s' not found.\n", varName + 1);
					}
				}
				j++;
			}
		}

		/* Does command line end with & */
		if (!strcmp(argv[i - 1], "&"))
		{
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

				// if we have <
				if (strcmp(argv[i], "<") == 0)
				{
					char *filename = argv[i + 1];
					int fd1 = open(filename, O_RDONLY, 0644);
					if (fd1 < 0)
					{
						perror("Error");
						return 1;
					}
					if (dup2(fd1, 0) < 0) // 0 == stdin
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
						// the first command: <first> | <second>
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
						// the second command: <first> | <second>
						int j = 0;
						int k = i + 1;
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
			wait(&status);
	}
}
