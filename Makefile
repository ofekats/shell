# Makefile for myshell

CC = gcc
CFLAGS = -Wall -Wextra -std=c99

myshell: last_shell.c
	$(CC) $(CFLAGS) -o myshell last_shell.c

.PHONY: clean

clean:
	rm -f myshell
