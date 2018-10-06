CC = c99
CFLAGS = -Wall -pedantic
SFLAGS = -lpthread

all:
	make server
	make client

server: 
	$(CC) server.c -o server $(CFLAGS) $(SFLAGS)

client: 
	$(CC) client.c -o client $(CFLAGS)

clean: 
	rm -f server client *.o
 
.PHONY: all clean 