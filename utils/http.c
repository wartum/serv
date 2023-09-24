#include "http.h"

#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static enum Method read_method(char* header)
{
    if (strncmp(header, "GET", 3) == 0)
    {
        return GET;
    }
    if (strncmp(header, "POST", 4) == 0)
    {
        return POST;
    }

    return GET;
}

static char* read_path(char* header)
{
    char* start = memchr(header, ' ', 10) + 2;
    if (start == NULL)
    {
        return NULL;
    }
    char* end = memchr(start, ' ', 256);
    if (end == NULL)
    {
        return NULL;
    }
    size_t size = end - start;
    char* path = malloc(size * sizeof(char) + 1);
    strncpy(path, start, size);
    path[size] = 0;
    if (size == 0)
    {
        strcpy(path, "index.html");
    }
    return path;
}

struct Request request_new(char* request, size_t request_len)
{
    struct Request result;

    for(int i = 0; i < request_len; i++)
    {
        if (request[i] == '\r' &&
            request[i + 1] == '\n' &&
            request[i + 2] == '\r' &&
            request[i + 3] == '\n')
        {
            result.header = malloc(i * sizeof(char));
            strncpy(result.header, request, i);
            result.header[i] = 0;

            i += 4;
            int body_size = request_len - i + 1;
            result.body = malloc(body_size * sizeof(char));
            strncpy(result.body, (request + i), body_size);
            break;
        }
    }

    result.method = read_method(result.header);
    result.path = read_path(result.header);

    return result;
}

void request_cleanup(struct Request* request)
{
    free(request->header);
    request->header = NULL;

    free(request->body);
    request->body = NULL;

    free(request->path);
    request->path = NULL;
}

struct Response response_new()
{
    struct Response response;
    response.body = NULL;
    response.header = NULL;
    response.status_code = OK;
    for (int i = 0; i < RESPONSE_HEADER_ATTRIBUTES_MAX; i++)
    {
        response.header_attribute[i] = NULL;
    }
    return response;
}

void response_cleanup(struct Response* response)
{
    free(response->body);
    response->body = NULL;

    free(response->header);
    response->header = NULL;

    for(int i = 0; i < RESPONSE_HEADER_ATTRIBUTES_MAX; i++)
    {
        if (response->header_attribute[i] != NULL)
        {
            free(response->header_attribute[i]);
            response->header_attribute[i] = NULL;
        }
    }
}

void response_body_from_file(struct Response* response, char* filepath)
{
    int buffer_size = 4096;
    response->body = malloc(buffer_size * sizeof(char));

    int fd = open(filepath, 0, O_RDONLY);
    if (fd < 0)
    {
        perror("Could not open a file");
        response->status_code = NOT_FOUND;
        return;
    }

    int cnt = read(fd, response->body, buffer_size);
    if (cnt <= 0)
    {
        perror("Could not read file");
        response->status_code = INTERNAL_ERROR;
        return;
    }
    close(fd);

    response->body[cnt] = 0;
    response->status_code = OK;
}

char* response_to_str(struct Response* response)
{
    int buffer_size = 4096;
    response->header = malloc(buffer_size * sizeof(char));

    // Compose header
    switch (response->status_code)
    {
    case NOT_FOUND:
        strcpy(response->header, "HTTP/1.1 404 Not Found\r\n");
        break;
    case OK:
        strcpy(response->header, "HTTP/1.1 200 Ok\r\n");
        break;
    case INTERNAL_ERROR:
    default:
        strcpy(response->header, "HTTP/1.1 500 Internal Error\r\n");
        break;
    }

    response->header_attribute[0] = malloc(50 * sizeof(char));
    strcpy(response->header_attribute[0], "Content-Type: text/html");

    for(int i = 0; i < RESPONSE_HEADER_ATTRIBUTES_MAX; i++)
    {
        if (response->header_attribute[i] != NULL)
        {
            size_t header_len = strlen(response->header);
            strcpy(response->header + header_len, response->header_attribute[i]);
            header_len = strlen(response->header);
            response->header[header_len + 0] = '\r';
            response->header[header_len + 1] = '\n';
            response->header[header_len + 2] = 0;
        }
    }

    // Compose body
    switch (response->status_code)
    {
    case OK:
        break;
    case NOT_FOUND:
        strcpy(response->body, "<h1>404 Not Found</h1>");
        break;
    case INTERNAL_ERROR:
    default:
        strcpy(response->body, "<h1>500 Internal Error</h1>");
        break;
    }

    int header_len = strlen(response->header);
    int body_len = strlen(response->body);
    char* result = malloc((header_len + body_len + 3) * sizeof(char));

    strcpy(result, response->header);
    result[header_len + 0] = '\r';
    result[header_len + 1] = '\n';
    strcpy(result + header_len + 2, response->body);
    return result;
}
