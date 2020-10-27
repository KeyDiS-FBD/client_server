all: bin bin/server bin/client
	
bin:
	mkdir bin

bin/server: src/server.c
	gcc src/server.c -o bin/server

bin/client: src/client.c
	gcc src/client.c -o bin/client

clean:
	rm bin/server bin/client
	rmdir bin

