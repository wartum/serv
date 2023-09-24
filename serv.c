#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
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

    struct Request request = request_new(buffer, cnt);
    free(buffer);

    struct Response response = response_new();
    response_body_from_file(&response, request.path);

    char* raw_response = response_to_str(&response);
    size_t raw_response_len = strlen(raw_response);

    send(client_fd, raw_response, raw_response_len, 0);

    request_cleanup(&request);
    response_cleanup(&response);
    free(raw_response);
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
