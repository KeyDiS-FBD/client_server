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

char *scan_word() {
    char ch;
    char *word = NULL;
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


char *scan_socket_word(int server, int *client_id) {
    char *word = NULL;

    int word_len = 0;
    if (read(server, client_id, 4) <= 0) {
        puts("Server closed");
        close(server);
        free(word);
        exit(0);
    }
    do {
        word = realloc(word, (word_len + 1) * sizeof(char));
        if (read(server, &word[word_len], 1) <= 0) {
            puts("Server closed");
            close(server);
            free(word);
            exit(0);
        }
        word_len++;
    } while (word[word_len - 1] != '\0');
    return word;
}

int check_argc(int argc) {
    if (argc != 3) {
        puts("Incorrect args.");
        puts("./client <ip> <port>");
        puts("Example:");
        puts("./client 127.0.0.1 5000");
        return 1;
    }
    return 0;
}

void server_recieve(int server) {
    char *word = NULL;
    int client_id;

    word = scan_socket_word(server, &client_id);
    while (strcmp(word, "exit") != 0) {
        printf("%d: %s\n", client_id, word);
        free(word);
        word = scan_socket_word(server, &client_id);
    }
    free(word);
    exit(0);
}

void terminal_recieve(int server) {
    char *word = NULL;

    do {
        free(word);
        word = scan_word();
        send_word(word, server);
    } while (strcmp(word, "exit") != 0);
    free(word);
    exit(0);
}

int main(int argc, char **argv) {
    if (check_argc(argc)) {
        return 1;
    }
    puts("Type \"exit\" to quit");

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int server = init_socket(ip, port);
    int pid;

    pid = fork();
    if (pid < 0) {
        perror("Error fork");
        exit(1);
    } else if (pid == 0) {
        server_recieve(server);
    } else {
        terminal_recieve(server);
    }
    wait(NULL);
    close(server);
    return 0;
}
