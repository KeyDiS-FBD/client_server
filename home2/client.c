#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_CONNECT
};

int init_socket(const char *ip, int port) {
    //open socket, result is socket descriptor
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Fail: open socket");
        exit(ERR_SOCKET);
    }

    //prepare server address
    struct hostent *host = gethostbyname(ip);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    memcpy(&server_address.sin_addr, host->h_addr_list[0],
           (socklen_t)sizeof(server_address.sin_addr));

    //connection
    if (connect(server_socket, (struct sockaddr*) &server_address,
        (socklen_t)sizeof(server_address)) < 0) {

        perror("Fail: connect");
        exit(ERR_CONNECT);
    }
    return server_socket;
}

char *scan_word(char *word) {
    char ch;
    puts("Wait word:");
    int i = 0;
    ch = getchar();
    while (ch != '\n') {
        word = realloc(word, (i + 1) * sizeof(char));
        word[i] = ch;
        i++;
        ch = getchar();
    }
    word = realloc(word, (i + 1) * sizeof(char));
    word[i] = '\0';
    return word;
}

void send_word(char *word, int server) {
    puts("Send data:");
    puts(word);
    int i = 0;
    while (word[i] != '\0') {
        write(server, &word[i], 1);
        i++;
    }
    write(server, &word[i], 1);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        puts("Incorrect args.");
        puts("./client <ip> <port>");
        puts("Example:");
        puts("./client 127.0.0.1 5000");
        return ERR_INCORRECT_ARGS;
    }
    char *ip = argv[1];
    int port = atoi(argv[2]);
    int server = init_socket(ip, port);

    char *word = NULL;
    word = scan_word(word);
    send_word(word, server);
    close(server);
    free(word);
    return OK;
}
