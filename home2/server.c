#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <init_socket.h>

void close_client_socket(int *client_socket, int clients_num) {
    for (int j = 0; j < clients_num; j++) {
        close(client_socket[j]);
    }
    free(client_socket);
}

int *accept_func(int clients_num, int server_socket) {
    struct sockaddr_in *client_address = malloc(clients_num * sizeof(struct sockaddr_in));
    int *client_socket = malloc(clients_num * sizeof(int));
    socklen_t size;
    for (int i = 0; i < clients_num; i++) {
        client_address[i].sin_family = AF_INET;
    }
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

int check_argc(int argc) {
    if (argc != 3) {
        puts("Incorrect args.");
        puts("./server <port> <number of clients>");
        puts("Example:");
        puts("./server 5000 3");
        return 1;
    }
    return 0;
}

void scan_socket_word(int *client_socket, int clients_num, int i) {
    char ch;
    while (1) {
        if (read(client_socket[i], &ch, 1) <= 0) {
            // perror("Error read");
            close_client_socket(client_socket, clients_num);
            printf("Client %d left\n", i + 1);
            exit(0);
        }
        printf("%d: %c\n", i + 1, ch);
    }
}

int main(int argc, char** argv) {
    if (check_argc(argc)) {
        exit(1);
    }
    int port = atoi(argv[1]);
    int clients_num = atoi(argv[2]);
    int server_socket = init_socket(NULL, port);
    int *client_socket = NULL;
    int pid;

    client_socket = accept_func(clients_num, server_socket);
    close(server_socket);

    puts("Recieve data:");
    for (int i = 0; i < clients_num; i++) {
        pid = fork();
        if (pid < 0) {
            perror("Error fork");
        } else if (pid == 0) {
            scan_socket_word(client_socket, clients_num, i);
        }
    }
    for (int i = 0; i < clients_num; i++) {
        wait(NULL);
    }
    close_client_socket(client_socket, clients_num);
    return 0;
}
