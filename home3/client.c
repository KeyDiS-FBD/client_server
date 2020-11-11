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


enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_CONNECT
};

struct data {
    int client_num;
    char word[26];
    char word_end;
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

        perror("Error: connect");
        exit(1);
    }
    return server_socket;
}

struct data scan_message() {
    char ch;
    struct data message;
    puts("Wait word:");
    int i = 0;
    do {
        ch = getchar();
        message.word[i] = ch;
        i++;
        if (i > 25) {
            perror("Error:too many characters, the word will be truncated");
            break;
        }
    } while (ch != '\n' && ch != ' ');
    message.word_end = ch;
    message.word[i - 1] = '\0';
    message.client_num = 0;
    return message;
}

void send_message(struct data message, int server) {
    // puts("Send data:");
    // puts(message.word);
    write(server, &message, sizeof(struct data));
}

// char *get_word(char *word, int server) {
//     int i = 0;
//     char ch;
//     read(server, &ch, 1);
//     while (ch != '\0') {
//         word = realloc(word, (i + 1) * sizeof(char));
//         word[i] = ch;
//         i++;
//         read(server, &ch, 1);
//     }
//     word = realloc(word, (i + 1) * sizeof(char));
//     word[i] = '\0';
//     return word;
// }

int main(int argc, char **argv) {
    if (argc != 3) {
        puts("Incorrect args.");
        puts("./client <ip> <port>");
        puts("Example:");
        puts("./client 127.0.0.1 5000");
        return ERR_INCORRECT_ARGS;
    }
    puts("Type \"exit\" to quit");

    struct data message;

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int server = init_socket(ip, port);

    void handler(int signo) {
        message.word[0] = 'e';
        message.word[1] = 'x';
        message.word[2] = 'i';
        message.word[3] = 't';
        message.word[4] = '\0';

        message.word_end = '\n';
        send_message(message, server);
        close(server);
        return 0;
    }
    signal(SIGINT, handler);

    do {
        message = scan_message();
        send_message(message, server);
    } while (strcmp(message.word, "exit") != 0);

    close(server);
    return OK;
}
