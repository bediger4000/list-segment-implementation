CC = gcc
CFLAGS = -g -Wall -Wextra

ls: ls.c
	$(CC) $(CFLAGS) -o ls ls.c
