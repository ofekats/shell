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
	printf("\nYou typed Control-C!\n");
}

// Structure to store variables
typedef struct
{
    char name[256]; 
    char value[1024];
} Variable;

Variable variables[100]; // Array to store variables
int variableCount = 0;   // Counter for the number of variables

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

int addVariable(char *name, char *value){
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
	int i, amper, retid, status;
	char *argv[10];
	char command[1024];
	char *token;
	char prompt[256] = "\x1b[35mHello\x1b[0m";
	char current_directory[1024];
	char last_command[1024] = "";

	signal(SIGINT, handler);
	while (1)
	{
		printf("\x1b[35m%s: \x1b[0m", prompt);
		fgets(command, 1024, stdin);
		command[strlen(command) - 1] = '\0'; // replace \n with \0
		if (strcmp(command, "exit") == 0 || strcmp(command, "exit()") == 0 || strcmp(command, "quit") == 0 )
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
            char *varName = command + 6;
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
				if (addVariable(varName, value) != 0)
				{
					printf("Too many variables, cannot add more.\n");
				}
			}
            continue;
        }

		else if (strchr(command, '$') != NULL)
        {
			char *charPtr = strchr(command, '$');
			char *equalSign = strchr(command, '=');
			if (equalSign != NULL)
			{
				// Variable assignment
				*equalSign = '\0'; // Split the string at '='
				char *varName = strtok(command, " ");
				char *varValue = equalSign + 2; // equalSign is \0, equalSign + 1 is space
				Variable *var = findVariable(varName+1);
				if (var != NULL)
				{
					// Variable already exists, update its value
					strncpy(var->value, varValue, sizeof(var->value) - 1);
					var->value[sizeof(var->value) - 1] = '\0'; // Ensure null-termination
				}
				else
				{
					// New variable, add it to the array
					if(addVariable(varName, varValue) != 0){
						printf("Too many variables, cannot add more.\n");
					}
				}
				continue;
			}
            // Variable substitution
			int index = charPtr - command; // the index of the char after the $ 
            char *varName = command + index + 1; // poiter to the char after the $ (name of the var)
            Variable *var = findVariable(varName);
            if (var != NULL)
            {
                printf("%s", var->value);
                continue;
            }
            else
            {
                printf("Variable '%s' not found.\n", varName);
                continue;
            }
			continue;
        }

		else if (strcmp(command, "!!") == 0)
		{
			// Repeat the last command
			if (strlen(last_command) > 0)
			{
				strcpy(command, last_command);
				printf("Repeating last command: %s\n", command);
			}
			else
			{
				printf("No previous command to repeat.\n");
				continue;
			}
		}
		strcpy(last_command, command);
		
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