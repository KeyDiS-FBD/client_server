CC=gcc
CFLAGS=-Wall -Werror -lm -g


.PHONY: all, clean

all: bin include/init_socket bin/server bin/client

include/init_socket: include/init_socket.c
	$(CC) $(CFLAGS) $@.c -c

%/server: %/server.c
	$(CC) $(CFLAGS) $@.c init_socket.o -o bin/server -Iinclude

%/client: %/client.c
	$(CC) $(CFLAGS) $@.c init_socket.o -o bin/client -Iinclude

%: bin include/init_socket %/server %/client
	#Success compile
	rm init_socket.o

bin:
	mkdir bin

bin/server: src/server.c
	$(CC) $(CFLAGS) src/server.c include/init_socket.o -o bin/server -Iinclude

bin/client: src/client.c
	$(CC) $(CFLAGS) src/client.c include/init_socket.o -o bin/client -Iinclude

clean:
	rm bin/server bin/client
	rmdir bin
