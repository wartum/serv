#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "utils/http.h"

int max_concurrent_connections = 20;
int server_fd = 0;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;

void initialize()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Could not bind socket to port number");
        exit(EXIT_FAILURE);
    }
}

void* handle_request(void* client_fd_ptr)
{
    int client_fd = *((int*)client_fd_ptr);
    char* rsp_header = NULL;
    char* rsp_body = NULL;
    char* buffer = NULL;
    size_t buffer_size = 4096;

    buffer = malloc(buffer_size * sizeof(char));
    int cnt = recv(client_fd, buffer, buffer_size, 0);
    if (cnt <= 0)
    {
        free(buffer);
        return NULL;
    }
    buffer[cnt] = 0;

    struct Request request = parse_request(buffer, cnt);
    free(buffer);

    int fd = open(request.path, 0, O_RDONLY);
    if (fd < 0)
    {
        perror("Could not open a file");
        rsp_header = response_header(404);
    }
    else
    {
        rsp_body = malloc(buffer_size * sizeof(char));
        cnt = read(fd, rsp_body, buffer_size);
        if (cnt <= 0)
        {
            perror("Could not read file");
            rsp_header = response_header(500);
        }
        else
        {
            rsp_body[cnt] = 0;
            rsp_header = response_header(200);
        }
        close(fd);
    }

    char response[buffer_size];
    memset(response, 0, buffer_size);

    strcpy(response, rsp_header);
    size_t header_len = strlen(rsp_header);
    response[header_len + 0] = '\r';
    response[header_len + 1] = '\n';
    header_len += 2;
    if (rsp_body != NULL)
    {
        strcpy(response + header_len, rsp_body);
    }
    size_t response_len = strlen(response);
    send(client_fd, response, response_len, 0);

    free(rsp_body);
    free_request(&request);
    close(client_fd);
    return NULL;
}

void listen_and_serve()
{
    if (listen(server_fd, max_concurrent_connections) < 0)
    {
        perror("Cannot listen with specified socket fd");
        exit(EXIT_FAILURE);
    }
    printf("Listening\n");

    while (1)
    {
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_fd < 0)
        {
            perror("Could not accept incoming request");
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, &handle_request, &client_fd);
        pthread_detach(thread_id);
    }
}

int main(void)
{
    initialize();
    listen_and_serve();
    return EXIT_SUCCESS;
}
