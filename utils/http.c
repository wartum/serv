#include <string.h>
#include "http.h"

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

struct Request parse_request(char* request, size_t request_len)
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

void free_request(struct Request* request)
{
    free(request->header);
    free(request->body);
    free(request->path);
}

char* response_header(int status_code)
{
    switch(status_code)
    {
    case 404:
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>";
    case 500:
        return "HTTP/1.1 500 Internal Error\r\nContent-Type: text/html\r\n\r\n<h1>500 Internal Error>";
    case 200:
    default:
        return  "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    }
}
