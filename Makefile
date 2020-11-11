CC=gcc
CFLAGS=-Wall -Werror -lm

.PHONY: all, clean

all: bin bin/server bin/client


%/server: %/server.c
	$(CC) $(CFLAGS) $@.c -o bin/server

%/client: %/client.c
	$(CC) $(CFLAGS) $@.c -o bin/client


%: bin %/server %/client
	echo "Success compile"

bin:
	mkdir bin

bin/server: src/server.c
	$(CC) $(CFLAGS) src/server.c -o bin/server

bin/client: src/client.c
	$(CC) $(CFLAGS) src/client.c -o bin/client

clean:
	rm bin/server bin/client
	rmdir bin
