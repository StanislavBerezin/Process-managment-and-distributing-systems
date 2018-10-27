CC = gcc
CFLAGS = -Wall -pedantic
SFLAGS = -pthread
FILE = 'none'
all:
	make server
	make client

server: 
	$(CC) server.c minesweeper.c -o server $(SFLAGS)

client: 
	$(CC) client.c -o client $(CFLAGS)

file: 
	$(CC) $(FILE).c -o $(FILE) $(CFLAGS) $(SFLAGS)

clean: 
	rm -f server client *.o
 
.PHONY: all clean 