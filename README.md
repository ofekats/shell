# Shell

This is a simple shell implementation written in C. It supports basic shell functionalities, including command execution, variable management, and input/output redirection.   
It also supports conditional execution with `if`, `then`, `else`, and `fi` statements.

## Features

- **Command Execution:** Execute commands with support for built-in commands and external programs.
- **Variable Management:** Create, read, and update shell variables.
- **Redirection:** Handle input and output redirection using `>`, `<`, `>>`, and `2>`.
- **Pipes:** Support for piping between commands using `|`.
- **Conditional Execution:** Execute commands based on the success or failure of previous commands using `if`, `then`, `else`, and `fi`.
- **Command History:** Repeat the last command using `!!`.
- **Prompt Customization:** Change the shell prompt using `prompt =`.

## Installation
After cloning the repository, do the following:
```bash
  make
  ./myshell
```
# Shell Program Documentation

You will be presented with a prompt where you can enter commands. Here are some example commands:

*   `echo $VARIABLE_NAME`: Print the value of an environment variable.
*   `prompt = new_prompt`: Change the shell prompt.
*   `cd /path/to/directory`: Change the current working directory.
*   `read variable_name`: Read input from stdin and assign it to a variable.
*   `variable=value`: Set a variable.
*   `variable=$variable_name`: Substitute the value of an environment variable in a command.
*   `command > file.txt`: Redirect output to a file.
*   `command < file.txt`: Redirect input from a file.
*   `command >> file.txt`: Append output to a file.
*   `command1 | command2`: Pipe the output of `command1` to `command2`.

### Conditional Execution

*   `if condition then command`: Execute `command` if the `condition` is true.
*   `else command`: Execute `command` if the `condition` is false.
*   `fi`: End of the conditional block.

### Signal Handling

*   `Ctrl-C` will display a message and return to the prompt.

## Contributors
*  <a href="https://github.com/MoriyaEster">Moriya Ester ohayon</a>
*  <a href="https://github.com/ofekats">Ofek Kats</a>
