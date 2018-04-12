#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define HTTP " HTTP/1.1"
#define NL "\r\n"
#define HOST "Host: "
#define IMS "If-Modified-Since: "

char *parse_url(char *url, int is_header, char *date, int *port);

char *format_date(char *date);

int main(int argc, char **argv)
{
    struct sockaddr_in address;
    int sock = 0, valread, is_header = 0, port = 80;
    struct sockaddr_in serv_addr;
    char *date = NULL, *url, buffer[1024] = {0};

    if (argc == 1) {
        printf("No URL given on commandline\n");
        return 1;
    } else if (argc == 2) {
        url = argv[1];
    } else if (argc == 3) {
        is_header = 1;
        url = argv[2];
    } else if (argc == 4) {
        date = argv[2];
        url = argv[3];
    } else {
        is_header = 1;
        date = argv[3];
        url = argv[4];
    }

    char *request = parse_url(url, is_header, date, &port);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed \n");
        return -1;
    }

    //send(sock , request, strlen(request) , 0 );
    printf("Message sent\n");

    //valread = read( sock , buffer, 1024);

    printf("%s\n", request);
    return 0;
}

char *parse_url(char *url, int is_header, char *date, int *port) {
    char *token = strtok_r(url, "://", &url), *method, *host, *path, *result, *for_date = NULL;
    int size;
    if (strcmp(token, "http") != 0) {
        printf("url needs to start with 'http'\n");
        exit(EXIT_FAILURE);
    }
    //deciding method
    method = (is_header) ? "HEAD " : "GET ";

    token = strtok_r(NULL, "/", &url);

    //checking to see if port number is included or not
    host = strtok_r(token, ":", &token);
    if (strlen(token) != 0) {
        *port = (int) strtol(token, (char **)NULL, 10);
    }

    //adding back '/'
    url -= sizeof(char);
    url[0] = '/';
    path = url;

    //method + path + HTTP version + crnl  + "Host: " + host + 2xcrnl + null terminator
    size = (int) (strlen(method) + strlen(path) + strlen(HTTP) + strlen(NL)
           + strlen(HOST) + strlen(host) + strlen(NL) + strlen(NL) + 1);
    if (date != NULL) {
        //strdup, free when finished
        for_date = format_date(date);
        size += (int) strlen(IMS) + strlen(for_date) + strlen(NL);
    }

    result = calloc(size, sizeof(char));

    //method
    strcat(result, method);
    strcat(result, path);
    strcat(result, HTTP);
    strcat(result, NL);
    //host
    strcat(result, HOST);
    strcat(result, host);
    strcat(result, NL);
    //date
    if (date != NULL) {
        strcat(result, IMS);
        strcat(result, for_date);
        strcat(result, NL);
        free(for_date);
    }
    strcat(result, NL);

    return result;
}

char *format_date(char *date) {
    int day, hour, min;
    char *result;
    time_t n_time;

    //day
    char *day_s = strtok_r(date, ":", &date);
    day = (int) strtol(day_s, (char **)NULL, 10);

    //hour
    char *hour_s = strtok_r(NULL, ":", &date);
    hour = (int) strtol(hour_s, (char **)NULL, 10);

    //min
    char *min_s = strtok_r(NULL, ":", &date);
    min = (int) strtol(min_s, (char **)NULL, 10);


    n_time=time(0);
    n_time=n_time-(day*24*3600+hour*3600+min*60);

    result = strdup(ctime(&n_time));
    return result;
}