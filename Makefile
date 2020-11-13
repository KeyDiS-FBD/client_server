CC=gcc
CFLAGS=-Wall -Werror -lm

.PHONY: all, clean

all: bin include/init_socket.o bin/server bin/client delete_object

delete_object:
	rm init_socket.o
include/init_socket: include/init_socket.c
	$(CC) $(CFLAGS) $@.c -c

%/server: %/server.c
	$(CC) $(CFLAGS) $@.c init_socket.o -o bin/server -Iinclude

%/client: %/client.c
	$(CC) $(CFLAGS) $@.c init_socket.o -o bin/client -Iinclude


%: bin %/server %/client  include/init_socket
	echo "Success compile"

bin:
	mkdir bin

bin/server: src/server.c
	$(CC) $(CFLAGS) src/server.c include/init_socket.o -o bin/server -Iinclude

bin/client: src/client.c
	$(CC) $(CFLAGS) src/client.c include/init_socket.o -o bin/client -Iinclude

clean:
	rm include/init_socket.o
	rm bin/server bin/client
	rmdir bin
