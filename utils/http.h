#include <stdlib.h>
#pragma once

enum Method
{
    GET,
    POST
};

struct Request
{
    char* header;
    char* body;
    enum Method method;
    char* path;
};

struct Request parse_request(char* request, size_t request_len);
void free_request(struct Request* request);
char* response_header(int status_code);
