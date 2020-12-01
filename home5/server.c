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

//Пользовательские заголовочные файлы
#include <init_socket.h>

#define OK 200
#define NOT_FOUND 404

int *socket_accept(int server_socket, int clients_num) {
    int *client_socket = malloc(clients_num * sizeof(int));
    struct sockaddr_in client_address;
    socklen_t size = sizeof(struct sockaddr_in);

    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = INADDR_ANY;
    for (int i = 0; i < clients_num; i++) {
        if ((client_socket[i] = accept(server_socket,
            (struct sockaddr *) &client_address,
            &size)) < 0) {

                perror("Error accept");
                exit(1);
            }
            printf("Connected:\n  ip:%s\n  port:%d\n",
            inet_ntoa(client_address.sin_addr),
            ntohs(client_address.sin_port));
    }
    return client_socket;
}

char *socket_scan_word(int client_socket, char *last_ch) {
    char *request = NULL;
    int request_len = 0;
    size_t bytes;

    do {
        if (read(client_socket, last_ch, 1) <= 0) {
            close(client_socket);
            free(request);
            exit(1);
        }
        if (*last_ch == '\0') {
            return NULL;
        }
        bytes = (request_len + 1) * sizeof(char);
        request = realloc(request, bytes);
        request[request_len] = *last_ch;
        request_len++;
    } while (*last_ch != ' ' && *last_ch != '\n');
    request[request_len - 1] = '\0';
    return request;
}

char **socket_scan_line(int client_socket, char *last_ch) {
    char **request = NULL;
    int request_len = 0;
    size_t bytes;

    do {
        bytes = (request_len + 1) * sizeof(char *);
        request = realloc(request, bytes);
        request[request_len] = socket_scan_word(client_socket, last_ch);
        request_len++;
    } while (*last_ch != '\0' && *last_ch != '\n');
    if (request[request_len - 1] != NULL) {
        bytes = (request_len + 1) * sizeof(char *);
        request = realloc(request, bytes);
        request[request_len] = NULL;

    }

    return request;
}

char ***http_parse_request(int client_socket) {
    char ***request = NULL;
    char last_ch = '\0';
    int request_len = 0;
    size_t bytes;

    do {
        bytes = (request_len + 1) * sizeof(char **);
        request = realloc(request, bytes);
        request[request_len] = socket_scan_line(client_socket, &last_ch);
        request_len++;
    } while (last_ch != '\0');
    bytes = (request_len + 1) * sizeof(char **);
    request = realloc(request, bytes);
    request[request_len] = NULL;

    return request;
}

// Functions for check correct input
void printLine(char **list) {
    int i = 0;
    while (list[i] != NULL) {
        printf("[%s] ", list[i]);
        i++;
    }
}

void printArrList(char ***list) {
    int i = 0;
    while (list[i] != NULL) {
        printLine(list[i]);
        puts("");
        i++;
    }
}

char *scan_file(FILE *fp) {
    char line[256];
    size_t max_line_size = 256;
    size_t line_size;
    char *filetext = NULL;
    size_t filetext_size = 0;

    while (fgets(line, max_line_size, fp) != NULL) {
        line_size = strlen(line);
        filetext_size += line_size;
        filetext = realloc(filetext, (filetext_size + 1) * sizeof(char));
        strncat(filetext, line, line_size);

    }
    filetext[filetext_size] = '\0';
    return filetext;
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

void send_response_404(int client_socket) {
    char *response_404 = "HTTP/1.1 404\n";

    printf("%s", response_404);
    send_word(response_404, client_socket);
}

void send_response(FILE *fp, int client_socket) {
    const char RESPONSE_TEMPLATE[] = {
        "HTTP/1.1 200\ncontent-type: html/text\ncontent-length: %d\n\n"
    };
    char *response = NULL;
    char *filetext = NULL;
    size_t response_size;
    int filetext_len = 0;
    // printf("status:%d\n", status);


    filetext = scan_file(fp);
    response_size = sizeof(RESPONSE_TEMPLATE);
    response = malloc(response_size);
    snprintf(response, response_size, RESPONSE_TEMPLATE, filetext_len);

    filetext_len = strlen(filetext);
    response_size += filetext_len * sizeof(char);
    response = realloc(response, response_size);

    strncat(response, filetext, filetext_len);
    printf("Response:\n%s", response);
    send_word(response, client_socket);
    free(response);
    free(filetext);
}

void get_http_request(int client_socket) {
    char ***request;
    FILE *fp = NULL;
    char *filename;

    while (1) {
        request  = http_parse_request(client_socket);
        filename = request[0][1];
        fp = fopen(filename, "rb");
        if (fp == NULL) {
            send_response_404(client_socket);
        } else {
            send_response(fp, client_socket);
        }
        free(request);
        free(fp);
    }

}

int main(int argc, char** argv) {
    // if (check_argc(argc)) {
    //     return 1;
    // }
    int port = atoi(argv[1]);
    int clients_num = atoi(argv[2]);
    int server_socket = init_socket(NULL, port);
    int *client_socket = NULL;
    int pid;

    if (port <= 0 || port >= 65535) {
        perror("Error bad port number");
        return 1;
    }


    client_socket = socket_accept(server_socket, clients_num);
    for (int client_index = 0; client_index < clients_num; client_index++) {
        pid = fork();
        if (pid == 0) {
            get_http_request(client_socket[client_index]);
        } else if (pid < 0) {
            perror("Error fork");
            exit(1);
        }
    }

    close(server_socket);
    free(client_socket);
    wait(NULL);
    return 0;
}
