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

void send_word(char *word, int client_id, int server) {
    int word_len = 0;
    if (write(server, &client_id, 4) <= 0) {
        close(server);
        free(word);
        exit(0);
    }
    do {
        if (write(server, &word[word_len], 1) <= 0) {
            close(server);
            free(word);
            exit(0);
        }
        word_len++;
    } while (word[word_len - 1] != '\0');
}

void close_client_socket(int *client_socket, int clients_num) {
    for (int j = 0; j < clients_num; j++) {
        send_word("exit", 0, client_socket[j]);
        close(client_socket[j]);
    }
    free(client_socket);
}

char *scan_socket_word(int *client_socket, int clients_num, int i) {
    char *word = NULL;
    int word_len = 0;
    do {
        word = realloc(word, (word_len + 1) * sizeof(char));
        if (read(client_socket[i], &word[word_len], 1) <= 0) {
            perror("Error read");
            close_client_socket(client_socket, clients_num);
            exit(1);
        }
        word_len++;
    } while (word[word_len - 1] != '\0');
    return word;
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


int main(int argc, char** argv) {
    if (check_argc(argc)) {
        return 1;
    }
    int port = atoi(argv[1]);
    if (port <= 0 || port >= 65535) {
        perror("Error bad port number");
        return 1;
    }

    int clients_num = atoi(argv[2]);
    int server_socket = init_socket(NULL, port);
    int *client_socket = NULL;
    char *word = NULL;
    int pid;

    // void handler(int signo) {
    //     // close_client_socket(client_socket, clients_num);
    //     // close(server_socket);
    //     // fflush(stdout);
    //     // free(word);
    //     exit(1);
    // }
    // signal(SIGINT, handler);

    client_socket = accept_func(clients_num, server_socket);
    puts("Recieve data:");
    for (int i = 0; i < clients_num; i++) {
        pid = fork();
        if (pid < 0) {
            perror("Error fork:");
            exit(1);
        } else if (pid == 0) {
            while (1) {
                word = scan_socket_word(client_socket, clients_num, i);
                if (strcmp(word, "exit") == 0) {
                    printf("Client %d left\n", i + 1);
                    free(word);
                    break;
                } else {
                    printf("%d: %s\n", i + 1, word);
                    for (int j = 0; j < clients_num; j++) {
                        if (i != j) {
                            send_word(word, i + 1, client_socket[j]);
                        }
                    }
                }
                fflush(stdout);
                free(word);
            }
            close_client_socket(client_socket, clients_num);
            // free(word);
            exit(0);
        }
    }
    for (int i = 0; i < clients_num; i++) {
        wait(NULL);
        close(client_socket[i]);
    }
    // close_client_socket(client_socket, clients_num);
    close(server_socket);
    free(client_socket);
    free(word);
    return 0;
}
