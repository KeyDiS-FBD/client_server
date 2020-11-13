#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int init_socket(const char *ip, int port) {
    //open socket, return socket descriptor
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Fail: open socket");
        exit(1);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);


    if (ip != NULL) {
        struct hostent *host = gethostbyname(ip);
        memcpy(&server_address.sin_addr, host->h_addr_list[0],
               (socklen_t)sizeof(server_address.sin_addr));

        //connection
        if (connect(server_socket, (struct sockaddr*) &server_address,
            (socklen_t)sizeof(server_address)) < 0) {

            perror("Error: connect");
            exit(1);
        }
    } else {
        server_address.sin_addr.s_addr = INADDR_ANY;
        int socket_option = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &socket_option,
                   sizeof(socket_option));
        if (server_socket < 0) {
            perror("Fail: set socket options");
            exit(1);
        }

        if (bind(server_socket, (struct sockaddr *) &server_address,
                 sizeof(server_address)) < 0) {

            perror("Fail: bind socket address");
            exit(1);
        }

        if (listen(server_socket, 5) < 0) {
            perror("Fail: bind socket address");
            exit(1);
        }
    }
    return server_socket;
}
