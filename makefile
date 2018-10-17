CC = cc
CFLAGS = -g -std=c99 -Werror -Wall
SFLAGS = -lpthread

all: server client

server: server.o
	$(CC) $(CFLAGS) -o server server.o

client: client.o
	$(CC) $(CFLAGS) -o client client.o

clean: 
	rm -f server client *.o
 
.PHONY: all clean 