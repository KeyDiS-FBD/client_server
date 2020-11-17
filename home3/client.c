#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

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
    int word_len = 0;
    ch = getchar();
    while (ch != '\n' && ch != ' ') {
        word = realloc(word, (word_len + 1) * sizeof(char));
        word[word_len] = ch;
        word_len++;
        ch = getchar();
    }
    word = realloc(word, (word_len + 1) * sizeof(char));
    word[word_len] = '\0';
    return word;
}



void send_word(char *word, int server) {
    int word_len = 0;
    do {
        if (write(server, &word[word_len], 1) <= 0) {
            close(server);
            free(word);
            exit(0);
        }
        word_len++;
    } while (word[word_len - 1] != '\0');
}


int main(int argc, char **argv) {
    if (argc != 3) {
        puts("Incorrect args.");
        puts("./client <ip> <port>");
        puts("Example:");
        puts("./client 127.0.0.1 5000");
        return 1;
    }
    puts("Type \"exit\" to quit");

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int server = init_socket(ip, port);
    char *word = NULL;

    void handler(int signo) {
        word = "exit";
        send_word(word, server);
        close(server);
        exit(0);
    }
    signal(SIGINT, handler);

    do {
        free(word);
        word = scan_word();
        send_word(word, server);
    } while (strcmp(word, "exit") != 0);

    close(server);
    free(word);
    return OK;
}
