#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <arpa/inet.h>

enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_SETSOCKETOPT,
    ERR_BIND,
    ERR_LISTEN
};

struct data {
    int client_num;
    char word[26];
    char word_end;
};

int init_socket(int port) {
    //open socket, return socket descriptor
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Fail: open socket");
        exit(ERR_SOCKET);
    }

    //set socket option
    int socket_option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &socket_option,
               sizeof(socket_option));
    if (server_socket < 0) {
        perror("Fail: set socket options");
        exit(ERR_SETSOCKETOPT);
    }

    //set socket address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    //Привязка сокета
    if (bind(server_socket, (struct sockaddr *) &server_address,
             sizeof(server_address)) < 0) {

        perror("Fail: bind socket address");
        exit(ERR_BIND);
    }

    //listen mode start
    if (listen(server_socket, 5) < 0) {
        perror("Fail: bind socket address");
        exit(ERR_LISTEN);
    }
    return server_socket;
}

int *accept_func(int clients_num, int server_socket) {
    struct sockaddr_in *client_address = malloc(clients_num * sizeof(struct sockaddr_in));
    int *client_socket = malloc(clients_num * sizeof(int));
    socklen_t size;
    for (int i = 0; i < clients_num; i++) {
        puts("Wait for connection");
        if ((client_socket[i] = accept(server_socket,
                    (struct sockaddr *) &client_address[i],
                    &size)) < 0) {
            perror("Error: accept");
            exit(1);
        }
        printf("Success connection, clients number: %d\n", i + 1);
        // printf("connected:\n ip:%s\n port:%d\n",
               // inet_ntoa(client_address.sin_addr),
               // ntohs(client_address.sin_port));
    }
    return client_socket;
}

// struct data socket_read(int client_socket) {
//     struct data message;
//     if (read(client_socket, &message, sizeof(struct data)) < 0) {
//         perror("Error: read");
//     }
//     close(client_socket);
//     exit(0);
//     return message;
// }

int main(int argc, char** argv) {
    if (argc != 3) {
        puts("Incorrect args.");
        puts("./server <port> <number of clients>");
        puts("Example:");
        puts("./server 5000 3");
        return ERR_INCORRECT_ARGS;
    }
    int port = atoi(argv[1]);
    if (port <= 0 || port >= 65535) {
        perror("Error: bad port number");
        return 1;
    }
    int clients_num = atoi(argv[2]);
    int server_socket = init_socket(port);
    int *client_socket = NULL;
    int pid;
    struct data message;
    puts("Recieve data:");
    while (1) {
        client_socket = accept_func(clients_num, server_socket);

        for (int i = 0; i < clients_num; i++) {
            pid = fork();
            if (pid < 0) {
                perror("Error: fork");
                exit(1);
            } else if (pid == 0) {
                while (1) {
                    do {
                        if (read(client_socket[i], &message,
                                 sizeof(struct data)) < 0) {

                            perror("Error read");
                            for (int j = 0; j < clients_num; j++) {
                                close(client_socket[j]);
                            }
                            free(client_socket);
                            return 1;
                        }
                        if (strcmp(message.word, "exit") == 0) {
                            break;
                        }
                        // message = socket_read(client_socket[i]);
                        printf("%d: %s\n", i + 1, message.word);
                        fflush(stdout);
                    } while (message.word_end != '\n');

                    if (strcmp(message.word, "exit") == 0) {
                        break;
                    }
                }
                exit(0);
            }

        }
        for (int i = 0; i < clients_num; i++) {
            wait(NULL);
        }
        for (int i = 0; i < clients_num; i++) {
            close(client_socket[i]);
        }
    }
    close(server_socket);
    free(client_socket);
    return OK;
}
