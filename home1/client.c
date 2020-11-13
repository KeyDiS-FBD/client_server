#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    ERR_CONNECT
};

char *scan_word() {
    char ch;
    char *word = NULL;
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

    puts("Type \"exit\" to quit");
    word = scan_word();
    while (strcmp(word, "exit") != 0) {
        send_word(word, server);
        free(word);
        word = scan_word();
    }
    if (strcmp(word, "exit") == 0) {
        puts("Client closed");
    }
    close(server);
    free(word);
    return OK;
}
