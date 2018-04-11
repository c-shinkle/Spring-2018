// Server side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
//#define RESPONSE_SIZE 25
#define FILE_NOT_FOUND "HTTP/1.1 404 Not Found\r\n"
#define FILE_FOUND "HTTP/1.1 200 OK\r\n"
#define FILE_NOT_MODIFIED "HTTP/1.1 304 Not Modified\r\n"

void read_input(int socket);

void display_response();

char *parse_client_response(char *src);

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    //char buffer[1024] = {0};
    char *hello = "I got the message!";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *) &address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    for (;;) {
        if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
                                 (socklen_t *) &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        //read_input(new_socket);
        char client_response_buffer[BUFFER_SIZE];
        read(new_socket, client_response_buffer, BUFFER_SIZE);

        char *server_response = parse_client_response(client_response_buffer);

        //display_response();

        send(new_socket, hello, strlen(hello), 0);
        printf("Message sent\n");
    }
}

void display_response() {
    char c;
    FILE *response = fopen("response", "r");
    if (response == NULL) {
        perror("reading file");
        exit(EXIT_FAILURE);
    }
    for (c = (char) fgetc(response); c != EOF; c = (char) fgetc(response))
        printf("%c", c);
    printf("\n");
    fclose(response);
}

void read_input(int socket) {
//    FILE *input = fopen("response", "w+");
//    if (input == NULL) {
//        perror("writing file");
//        exit(EXIT_FAILURE);
//    }
    char reading_buffer[BUFFER_SIZE];
    int chars_read = read(socket, reading_buffer, BUFFER_SIZE);
    //fprintf(input, "%s", reading_buffer);
//    do {
//        chars_read = read(socket, reading_buffer, BUFFER_SIZE);
//        if (chars_read < 0) {
//            perror("writing file");
//            exit(EXIT_FAILURE);
//        }
//        fprintf(input, "%s", reading_buffer);
//    } while (chars_read > 0);
    //fclose(input);
}

char *load_file_contents(int mode, char *filename) {
    int status_size = 0;
    if (mode == 1) {
        status_size = strlen(FILE_FOUND);
    } else if (mode == 0) {
        status_size = strlen(FILE_NOT_MODIFIED);
    }

    char * buffer = 0;
    long length;
    FILE * f = fopen (filename, "rb");

    if (f)
    {
        fseek (f, 0, SEEK_END);
        length = ftell (f) + status_size;
        fseek (f, 0, SEEK_SET);
        buffer = malloc (length);
        if (buffer)
        {
            if (mode == 1) {
               strcpy(buffer, FILE_FOUND);
            } else if (mode == 0) {
                strcpy(buffer, FILE_NOT_MODIFIED);
            }
            buffer += status_size;
            fread (buffer, 1, length, f);
        }
        fclose (f);
    }

    return buffer;
}

char *parse_client_response(char *src) {
    char *method, *url, *version, *parse, *result = NULL;

    parse = strtok_r(src, " ", &src);
    method = strdup(parse);

    parse = strtok_r(NULL, " ", &src);
    url = strdup(parse);

    parse = strtok_r(NULL, " ", &src);
    version = strdup(parse);

    FILE *f = fopen(url, "r");
    if (f != NULL) {
        fclose(f);
        if (strcmp(method, "GET") == 0)
            result = load_file_contents(0, url);
        else if (strcmp(method, "HEAD") == 0)
            result = strdup(FILE_FOUND);
        //TODO
        //else if (strcmp(method, "GET MODIFIED") == 0)
    } else {
        result = strdup(FILE_NOT_FOUND);
    }

    free(method);
    free(url);
    free(version);

    return result;
}