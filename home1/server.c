#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <init_socket.h>

enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_SETSOCKETOPT,
    ERR_BIND,
    ERR_LISTEN
};

int *accept_func(int clients_num, int server_socket) {
    struct sockaddr_in *client_address = malloc(clients_num * sizeof(struct sockaddr_in));
    int *client_socket = malloc(clients_num * sizeof(int));
    socklen_t size;
    size = sizeof(struct sockaddr_in);
    for (int i = 0; i < clients_num; i++) {
        puts("Wait for connection");
        if ((client_socket[i] = accept(server_socket,
                    (struct sockaddr *) &client_address[i],
                    &size)) < 0) {
            perror("Error: accept");
            exit(1);
        }
        printf("Connected:\n   ip: %s\n   port: %d\n   client number: %d\n",
               inet_ntoa(client_address[i].sin_addr),
               ntohs(client_address[i].sin_port),
               i + 1);
    }
    free(client_address);
    return client_socket;
}

void close_client_socket(int *client_socket, int clients_num) {
    for (int j = 0; j < clients_num; j++) {
        close(client_socket[j]);
    }
    free(client_socket);
}

void scan_socket_word(int *client_socket, int clients_num) {
    char ch;
    for (int i = 0; i < clients_num; i++) {
        if (read(client_socket[i], &ch, 1) <= 0) {
            printf("Client %d closed\n", i + 1);
            // perror("Error read");
            close_client_socket(client_socket, clients_num);
            exit(1);
        }
        printf("%d: %c\n", i + 1, ch);
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        puts("Incorrect args.");
        puts("./server <port> <number of clients>");
        puts("Example:");
        puts("./server 5000 3");
        return ERR_INCORRECT_ARGS;
    }
    int port = atoi(argv[1]);
    int clients_num = atoi(argv[2]);
    int server_socket = init_socket(NULL, port);
    int *client_socket = NULL;

    client_socket = accept_func(clients_num, server_socket);
    close(server_socket);

    puts("Recieve data:");
    while(1) {
        scan_socket_word(client_socket, clients_num);
    }
    return OK;
}
