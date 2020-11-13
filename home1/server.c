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
    int *client_socket = malloc(clients_num * sizeof(int));
    char ch;

    while(1) {
        // close(server_socket);
        puts("Wait for connection");
        struct sockaddr_in client_address;
        client_address.sin_family = AF_INET;

        socklen_t size;
        for (int i = 0; i < clients_num; i++) {
            client_socket[i] = accept(server_socket,
                        (struct sockaddr *) &client_address,
                        &size);
            printf("connected:\n ip:%s\n port:%d\n",
                   inet_ntoa(client_address.sin_addr),
                   ntohs(client_address.sin_port));
        }
        puts("Recieve data:");
        while(1) {
            for (int i = 0; i < clients_num; i++) {
                if (read(client_socket[i], &ch, 1) < 0) {
                    perror("Error write");
                    for (int i = 0; i < clients_num; i++) {
                        close(client_socket[i]);
                    }
                    free(client_socket);
                    return 1;
                }
                printf("%d: %c\n", i + 1, ch);
            }
        }
        for (int i = 0; i < clients_num; i++) {
            close(client_socket[i]);
        }
        free(client_socket);
    }


    return OK;
}
