#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <pthread.h>

#define PORT 8949
#define BUFFER_SIZE 1024

#define FILE_NOT_FOUND "HTTP/1.1 404 Not Found\r\n"
#define FILE_FOUND "HTTP/1.1 200 OK\r\n"
#define NOT_MOD "HTTP/1.1 304 Not Modified\r\n"
#define IF_MOD "If-Modified-Since:"
#define BAD_REQ "HTTP/1.1 400 Bad Request\r\n"

typedef struct {
    int year, month, day, hour, min;
}mytime_t;

char *load_file_contents(char *filename);

int get_month(char *m);

char *client_response_helper(int is_get, char *modified, char *date, char* filename);

char *parse_client_response(char *src);

void parse_time(mytime_t *my, char *date);

int calc_diff(char *date, char *filename);

void *doit(void *new_socket);

int main(int argc, char **argv) {
    struct sockaddr_in address;
    int server_fd, opt = 1, addrlen = sizeof(address), new_socket;

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
        pthread_t thread;
        pthread_create(&thread, NULL, doit, &new_socket);
    }
}

char *load_file_contents(char *filename) {
    int status_size = (int) strlen(FILE_FOUND);
    char *buffer = NULL;
    long length;
    FILE * f = fopen (filename, "rb");
    if (f != NULL) {
        fseek (f, 0, SEEK_END);
        length = ftell(f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc(length+status_size+1);
        if (buffer != NULL) {
            strcpy(buffer, FILE_FOUND);
            fread(buffer + (sizeof(char)*status_size), sizeof(char), length, f);
            buffer[length + status_size] = 0;
        }
        fclose (f);
    }
    return buffer;
}

int get_month(char *m) {
    if (strcmp(m, "Jan") == 0) {
        return 1;
    } else if (strcmp(m, "Feb") == 0) {
        return 2;
    } else if (strcmp(m, "Mar") == 0) {
        return 3;
    } else if (strcmp(m, "Apr") == 0) {
        return 4;
    } else if (strcmp(m, "May") == 0) {
        return 5;
    } else if (strcmp(m, "Jun") == 0) {
        return 6;
    } else if (strcmp(m, "Jul") == 0) {
        return 7;
    } else if (strcmp(m, "Aug") == 0) {
        return 8;
    } else if (strcmp(m, "Sep") == 0) {
        return 9;
    } else if (strcmp(m, "Oct") == 0) {
        return 10;
    } else if (strcmp(m, "Nov") == 0) {
        return 11;
    } else if (strcmp(m, "Dec") == 0) {
        return 12;
    } else {
        return -1;
    }
}

char *client_response_helper(int is_get, char *modified, char *date, char* filename) {
    if (modified != NULL) {
        if (strcmp(modified, IF_MOD) == 0) {
            int diff = calc_diff(date, filename);
            if (diff == -1) {
                return (is_get) ? load_file_contents(filename) : strdup(FILE_FOUND);
            } else {
                return strdup(NOT_MOD);
            }
        } else {
            return strdup(BAD_REQ);
        }
    } else {
        return (is_get) ? load_file_contents(filename) : strdup(FILE_FOUND);
    }
}

char *parse_client_response(char *src) {
    char *method, *filename, *version, *parse, *modified = NULL,
            *line, *line_dup, *line_free, *result = NULL, *date = NULL, *date_free;

    line = strtok_r(src, "\r\n", &src);
    line_dup = line_free = strdup(line);

    strtok_r(NULL, "\r\n", &src);

    if ((line = strtok_r(NULL, "\r\n", &src)) != NULL) {
        if((modified = strtok_r(line, " ", &line)) != NULL) {
            parse = strtok_r(NULL, "\r\n", &line);
            date = date_free = strdup(parse);
        }
    }

    parse = strtok_r(line_dup, " ", &line_dup);
    method = strdup(parse);

    parse = strtok_r(NULL, " ", &line_dup);
    filename = strdup(parse + sizeof(char));

    parse = strtok_r(NULL, " ", &line_dup);
    version = strdup(parse);

    FILE *f = fopen(filename, "r");
    if (f != NULL) {
        fclose(f);
        if (strcmp(method, "GET") == 0) {
            result = client_response_helper(1, modified, date, filename);
        } else if (strcmp(method, "HEAD") == 0) {
            result = client_response_helper(0, modified, date, filename);
        } else {
            result = strdup(BAD_REQ);
        }
    } else {
        result = strdup(FILE_NOT_FOUND);
    }

    free(method);
    free(filename);
    free(version);
    free(line_free);
    if (date != NULL)
        free(date_free);

    return result;
}

void parse_time(mytime_t *my, char *date) {
    int year, month, day, hour, min;
    //weekday
    strtok_r(date, " ", &date);

    //month
    char *month_s = strtok_r(NULL, " ", &date);
    month = get_month(month_s);

    //day
    char *day_s = strtok_r(NULL, " ", &date);
    day = (int) strtol(day_s, (char **)NULL, 10);

    //hour
    char *hour_s = strtok_r(NULL, ":", &date);
    hour = (int) strtol(hour_s, (char **)NULL, 10);

    //min
    char *min_s = strtok_r(NULL, ":", &date);
    min = (int) strtol(min_s, (char **)NULL, 10);

    //sec
    strtok_r(date, " ", &date);

    //year
    char *year_s = strtok_r(NULL, " ", &date);
    year = (int) strtol(year_s, (char **)NULL, 10);

    my->year = year;
    my->month = month;
    my->day = day;
    my->hour = hour;
    my->min = min;
}

int calc_diff(char *date, char *filename) {
    mytime_t since;
    mytime_t file;

    parse_time(&since, date);

    struct stat fileStat;
    if (stat(filename, &fileStat) < 0) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    time_t last_mod = fileStat.st_mtime;

    char tmp[30];
    strcpy(tmp, ctime(&last_mod));
    parse_time(&file, tmp);

    if (since.year < file.year) {
        return -1;
    } else if (since.year > file.year) {
        return 1;
    } else {
        if (since.month < file.month) {
            return -1;
        } else if (since.month > file.month) {
            return 1;
        } else {
            if (since.day < file.day) {
                return -1;
            } else if (since.day > file.day) {
                return 1;
            } else {
                if (since.hour < file.hour) {
                    return -1;
                } else if (since.hour > file.hour) {
                    return 1;
                } else {
                    if (since.min < file.min) {
                        return -1;
                    } else if (since.min > file.min) {
                        return 1;
                    } else {
                        return 0;
                    }
                }
            }
        }
    }
}

void *doit(void *p) {
    int *ptr = (int *) p;
    int new_socket = *ptr;
    char client_response_buffer[BUFFER_SIZE] = {0};
    read(new_socket, client_response_buffer, BUFFER_SIZE);

    printf("Message received\n");

    //Need to free pointer
    char *server_response = parse_client_response(client_response_buffer);

    send(new_socket, server_response, strlen(server_response), 0);
    printf("Message sent\n");

    free(server_response);
}